#include <N76E003.h>
#include <adc.h>
#include <i2c.h>
#include <iap.h>
#include <irq.h>
#include <tick.h>
#include <uart.h>
#include <terminal.h>

#include <dump.h>
#include <dht.h>
#include <bv4618.h>
#include <ds3231.h>
#include <pcf8574.h>
#include <i2c_mem.h>

#include "main.h"
#include "cfg.h"
#include "ps2k.h"

#define APP_VERSION "2104.24"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
#ifdef MEM_DEBUG
	"imem\n"
	"cmem [$addr [$len]]\n" /* cmem x4780 to print last page */
	"sfr [$addr]\n"			/* dump all SFRs or the specified one */
#endif
	"reset\n"						 /* sw reset */
	"delay $val [mks]\n"			 /* delay milli/micro second and generate markers */
	"rctrim save\n"					 /* save new RC trim value */
	"rctrim [$trim]\n"				 /* apply new RC trim value */
	"i2c scan|stop\n"				 /* scan I2C bus for valid addresses */
	"i2c wr $dev $val [$val ...]\n"  /* write data */
	"i2c read $dev $addr [$len]\n"	 /* read data with re-start */
	"i2cmem erase [$fill]\n"		 /* erase EEPROM memory using fill character, 0xFF by default */
	"i2cmem read $addr\n"			 /* read one byte from i2c EEPROM memory */
	"i2cmem write $addr $val\n"		 /* write one byte to i2c EEPROM memory */
	"i2cmem dump [$addr] [$len]\n"	 /* dump i2c EEPROM memory */
	"dht\n"							 /* read DHT sensor */
	"kbd $cmd [$arg]\n"
	"timer on|off\n"		  /* print info every second */
	"timer lcd|uart on|off\n" /* toggle LCD/UART output */
#ifdef USE_BV4618_LCD /* use BV4618 controller for 4x20 LCD character display */
	"bv cls\n"		  /* clear screen */
	"bv clr\n"		  /* clear line to the right */
	"bv init\n"
	"bv reset\n"
	"bv led on|off\n"
	"bv goto 1-4 [1-20]\n"
#endif
#ifdef USE_PCF8574_LCD
	"pcf cls\n"
	"pcf init\n"
	"pcf led on|off\n"
	"pcf cursor on|off\n"
	"pcf goto $line [$pos]\n"
#endif
	"rtc init\n"
	"rtc dump\n"
	"rtc time [hh:mm:ss]\n"
	"rtc temp\n"
	"rtc 32k on|off\n"
	"rtc sqw on|off|1hz|1k|4k|8k";

static void rtc_print_time(void);
static void rtc_print_temperature(void);

#ifndef STORE_CMD_TO_I2CMEM
#define STORE_CMD_TO_I2CMEM 0
#endif

static uint8_t icmd;

int8_t test_cli(__idata char *cmd)
{
	uint8_t i, reg, n;
	/* too many variables, put some of them to idata segment */
	__idata uint16_t len;
	__idata uint16_t addr;
	__idata char *arg = get_arg(cmd);

#if STORE_CMD_TO_I2CMEM
	/* write commands history to i2c EEPROM */
	addr = icmd * CMD_LEN;
	if (addr >= I2C_MEM_SIZE) {
		addr = 0;
		icmd = 0;
	}
	i2cmem_write_data(addr, cmd, CMD_LEN);
	icmd += 1;
#endif

	if (str_is(cmd, "help")) {
		uart_putsc("VER: ");
		uart_putsc(APP_VERSION);
		uart_putsc(" (");
		uart_putn(sdcc_get_code_size());
		uart_putsc(" bytes)\n");

		uart_putsc("CID: ");
		uart_puth(iap_read_cid());
		uart_putsc("\nDID: ");
		uart_puth(iap_read_did(0));
		uart_puth(iap_read_did(1));
		uart_putsc("\nCFG: ");
		for (i = 0; i < IAP_CFG_SIZE; i++) {
			if (i)
				uart_putc(' ');
			uart_puth(iap_read_cfg(i));
		}
		uart_putc('\n');

		uart_putsc("BGP: "); /* ADC bandgap */
		uart_puthw(get_vdd(ADC_GET_RAW_BGAP));
		uart_putc(' ');
		uart_putn(get_vdd(ADC_GET_BGAP));
		uart_putsc(" mV\n");
		uart_putsc("Vdd: ");
		uart_putn(get_vdd(ADC_GET_VDD));
		uart_putsc(" mV\nUID: ");
		for (i = 0; i < IAP_UID_SIZE; i++) {
			if (i)
				uart_putc('-');
			uart_puth(iap_read_uid(i));
		}
		uart_putc('\n');
		uart_putsc("CMD:");
		__code char *list = cmd_list;
		for (;; list++) {
			uint8_t ch = *list;
			if (ch == 0)
				break;
			if (ch == '\n') {
				uart_putc('\n');
				uart_putsc("    ");
				continue;
			}
			uart_putc(ch);
		}
		uart_putc('\n');
		goto EOK;
	}

	if (str_is(cmd, "reset")) {
		cfg_save();
		uart_putsc("\nresetting...\n");
		while (!uart_tx_empty());
		RCTRIM0 = trim0;
		RCTRIM1 = trim1;
		sw_reset();
	}

	/* using MARK to measure execution interval on oscilloscope */
	if (str_is(cmd, "delay")) {
		len = argtou(arg, &arg);
		if (str_is(arg, "mks")) {
			MARK; /* generate start pulse */
			delay_mks(len);
			MARK; /* generate stop pulse */
		} else {
			MARK; /* generate start pulse */
			delay(len);
			MARK; /* generate stop pulse */
		}
		goto EOK;
	}

#ifdef MEM_DEBUG
	if (str_is(cmd, "cmem")) {
		/* by default - dump all memory */
		addr = 0;
		len = APROM_SIZE;
		if (*arg != '\0') {
			/* start address */
			addr = argtou(arg, &arg);
			if (addr >= APROM_SIZE)
				goto EARG;
			/* by default - dump page if start address is specified */
			len = 128;
			if (*arg != '\0')
				len = argtou(arg, &arg);
			/* check for valid length */
			if (len == 0)
				len = 128;
			if ((addr + len) > APROM_SIZE)
				len = APROM_SIZE - addr;
		}
		dump_header();
		for (uint16_t n = 0; n < len; n += 16) {
			dump_set_bufc((__code uint8_t *)addr, 16);
			dump_xbuf(addr, 16);
			addr += 16;
		}
		event_flush(); /* printing takes too long, so clear timer events */
		goto EOK;
	}

	if (str_is(cmd, "imem")) {
		arg = 0;
		dump_header();
		while (1) {
			dump_set_bufi(arg, 16);
			dump_xbuf((uint16_t)arg, 16);
			if (arg == (__idata char *)0xF0)
				break;
			arg += 16;
		}
		goto EOK;
	}

	if (str_is(cmd, "sfr")) {
		uint8_t sfr;
		if (*arg) {
			sfr = argtou(arg, &arg);
			if (sfr >= 0x80) {
				i = sfr_read(sfr);
				uart_puth(i);
				uart_putc(' ');
				sfr_page(1);
				i = sfr_read(sfr);
				uart_puth(i);
				uart_putc('\n');
				sfr_page(0);
				goto EOK;
			}
			goto EARG;
		}

		/* print page 0 */
		dump_header();
		for (i = 0; i <= 0x7F; i++) {
			xbuf[i & 0x0F] = sfr_read(i + 0x80);
			if ((i & 0x0F) == 0x0F)
				dump_xbuf((i & 0xF0) | 0x80, 16);
		}
		uart_putsc(" page 1:\n");
		/* print page 1 */
		for (i = 0; i <= 0x7F; i++) {
			sfr_page(1);
			xbuf[i & 0x0F] = sfr_read(i + 0x80);
			sfr_page(0);
			if ((i & 0x0F) == 0x0F)
				dump_xbuf((i & 0xF0) | 0x80, 16);
		}
		goto EOK;
	}
#endif

	if (str_is(cmd, "rctrim")) {
		if (str_is(arg, "save"))
			cfg_save();
		else if (*arg != '\0') {
			trim = (uint8_t)argtou(arg, &arg);
			cfg.trim = trim;
			set_rc_trim(trim);
		} else {
			uart_puth(trim0);
			uart_putsc("+");
			uart_putbit(trim1);
			uart_putsc(" - ");
			uart_putn(trim);
			uart_putsc(" = ");
			uart_puth(RCTRIM0);
			uart_putsc("+");
			uart_putbit(RCTRIM1);
			uart_putc('\n');
		}
		goto EOK;
	}

	if (str_is(cmd, "i2c")) {
		if (str_is(arg, "scan")) {
			for (i = 0x08; i < 0x7F; i++) {
				if (i2c_start(i << 1) == I2C_EOK) {
					uart_putsc("Found device: 0x");
					uart_puth(i);
					uart_putsc(" (0x");
					uart_puth(i << 1);
					uart_putsc(")");
					uart_putln();
				}
				i2c_stop();
			}
			goto EOK;
		}
		if (str_is(arg, "stop")) {
			i2c_stop();
			goto EOK;
		}
		if (str_is(arg, "wr")) { /* write data */
			arg = get_arg(arg);
			i = argtou(arg, &arg);
			if (i2c_start(i << 1) == I2C_EOK) {
				while (*arg != '\0')
					i2c_write(argtou(arg, &arg));
			}
			i2c_stop();
			goto EOK;
		}
		if (str_is(arg, "read")) { /* read byte */
			arg = get_arg(arg);
			i = argtou(arg, &arg);
			if (*arg == '\0')
				goto EARG;
			reg = argtou(arg, &arg);
			n = argtou(arg, &arg);
			if (n == 0)
				n = 1;
			if (i2c_start(i << 1) == I2C_EOK) {
				i2c_write(reg);
				if (i2c_start((i << 1) | I2C_READ) == I2C_EOK) {
					while(n) {
						i = i2c_read(n > 1);
						uart_putsc(" x");
						uart_puth(i);
						n--;
					}
				}
			}
			i2c_stop();
			uart_putln();
			goto EOK;
		}
		goto EARG;
	}

	if (str_is(cmd, "i2cmem")) {
		if (str_is(arg, "read")) { /* read 1 byte */
			arg = get_arg(arg);
			addr = argtou(arg, &arg);
			if (addr >= I2C_MEM_SIZE)
				goto EARG;
			i2cmem_read_data(addr, &i, 1);
			uart_puthl(i);
			goto EOK;
		}
		if (str_is(arg, "dump")) { /* dump i2c memory */
			arg = get_arg(arg);
			addr = argtou(arg, &arg);
			if (addr > (I2C_MEM_SIZE - XBUF_SIZE))
				addr = I2C_MEM_SIZE - XBUF_SIZE;
			addr &= 0xFFF0; /* align to 16 byte offset */

			len = 0;
			if (*arg)
				len = argtou(arg, &arg);
			if (len == 0)
				len = I2C_MEM_SIZE - addr;
			dump_header();
			for (; len >= XBUF_SIZE; addr += XBUF_SIZE) {
				i2cmem_read_xdata(addr, xbuf, XBUF_SIZE);
				dump_xbuf(addr, XBUF_SIZE);
				len -= XBUF_SIZE;
			}
			if (len) {
				i2cmem_read_xdata(addr, xbuf, len);
				dump_xbuf(addr, len);
			}
			goto EOK;
		}
		if (str_is(arg, "write")) { /* write one byte */
			arg = get_arg(arg);
			addr = argtou(arg, &arg);
			i = argtou(arg, &arg);
			i2cmem_write_byte(addr, i);
			goto EOK;
		}
		if (str_is(arg, "erase")) {
			len = 0xFF;
			arg = get_arg(arg);
			if (*arg)
				len = argtou(arg, &arg);
			for (i = 0; i < CMD_LEN; i++)
				cmd[i] = len;
			for (addr = 0; addr < I2C_MEM_SIZE; addr += CMD_LEN)
				i2cmem_write_data(addr, cmd, CMD_LEN);
			return CLI_ENOHIST;
		}
		goto EARG;
	}

#ifdef USE_PCF8574_LCD
	if (str_is(cmd, "pcf")) {
		if (str_is(arg, "init")) {
			pcf_init();
			goto EOK;
		}
		if (str_is(arg, "cls")) {
			pcf_cls();
			goto EOK;
		}
		if (str_is(arg, "led")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				pcf_led_enable(true);
			else if (str_is(arg, "off"))
				pcf_led_enable(false);
			else
				goto EARG;
			goto EOK;
		}
		if (str_is(arg, "cursor")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				pcf_cursor_enable(true);
			else if (str_is(arg, "off"))
				pcf_cursor_enable(false);
			else
				goto EARG;
			goto EOK;
		}
		if (str_is(arg, "goto")) {
			arg = get_arg(arg);
			i = argtou(arg, &arg);
			if ((i < 1) || (i > PCF8574_LINES))
				goto EARG;
			uint8_t col = 1;
			if (*arg)
				col = argtou(arg, &arg);
			if ((col < 1) || (col > PCF8574_CHARS))
				goto EARG;
			pcf_goto(i - 1, col - 1);
			goto EOK;
		}
		pcf_puts(arg);
		goto EOK;
	}
#endif

#ifdef USE_BV4618_LCD
	if (str_is(cmd, "bv")) {
		if (str_is(arg, "init")) {
			bv4618_set_lines(4);
			bv4618_set_columns(20);
			bv4618_disable_scroll(1);
			bv4618_cls();
			delay(100);
			bv4618_hide();
			goto EOK;
		}
		if (str_is(arg, "reset")) {
			bv4618_reset();
			delay(500);
			goto EOK;
		}
		if (str_is(arg, "cls")) {
			bv4618_cls();
			delay(100);
			goto EOK;
		}
		if (str_is(arg, "clr")) {
			bv4618_clright();
			goto EOK;
		}
		if (str_is(arg, "goto")) {
			arg = get_arg(arg);
			i = argtou(arg, &arg);
			if ((i < 1) || (i > 4))
				goto EARG;
			uint8_t col = 1;
			if (*arg)
				col = argtou(arg, &arg);
			if ((col < 1) || (col > 20))
				goto EARG;
			bv4618_goto(i - 1, col - 1);
			goto EOK;
		}
		if (str_is(arg, "led")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				bv4618_led(1);
			else if (str_is(arg, "off"))
				bv4618_led(0);
			else
				goto EARG;
			goto EOK;
		}
		bv4618_puts(arg);
		goto EOK;
	}
#endif

	if (str_is(cmd, "dht")) {
		i = dht_read();
		if (i == DHT_OK) {
			uart_putsc("RH: ");
			uart_putn(dht.data.rh / 10);
			uart_putc('.');
			uart_putn(dht.data.rh % 10);
			uart_putsc(" %\n");
			uart_putsc("T : ");
			uart_putn(dht.data.tc / 10);
			uart_putc('.');
			uart_putn(dht.data.tc % 10);
			uart_putsc(" C\n");
		} else if (i == DHT_ERR_CRC)
			uart_putsc("CRC error\n");
		else
			uart_putsc("device timeout\n");
		goto EOK;
	}

	if (str_is(cmd, "kbd")) {
		i = argtou(arg, &arg);
		kbd_send_cmd(i);
		if (*arg) {
			i = millis8();
			while(kbd_cmd_pending()) {
				if (elapsed(i) > 100)
					return CLI_ENODEV;
			}
			i = argtou(arg, &arg);
			kbd_send_cmd(i);
		}
		i = millis8();
		while (kbd_cmd_pending()) {
			if (elapsed(i) > 100)
				return CLI_ENODEV;
		}
		goto EOK;
	}

	if (str_is(cmd, "rtc")) {
		uint8_t data;
		if (str_is(arg, "init")) {
			ds3231_init();
			goto EOK;
		}
		if (str_is(arg, "dump")) {
			if (ds3231_rw(0, DS3231_BUF_SIZE, I2C_READ) == I2C_EOK) {
				/* print index header */
				for (i = 0; i < DS3231_BUF_SIZE; i++) {
					if (i < 10)
						uart_putc(' ');
					uart_putn(i);
					uart_putc(' ');
				}
				uart_putsc("\n");
				/* print data itself */
				for (i = 0; i < DS3231_BUF_SIZE; i++) {
					data = ds3231[i];
					uart_puth(data);
					uart_putc(' ');
				}
				uart_putsc("\n");
				goto EOK;
			}
			return CLI_ENODEV;
		}
		if (str_is(arg, "temp")) {
			rtc_print_temperature();
			uart_putln();
			goto EOK;
		}
		if (str_is(arg, "time")) {
			arg = get_arg(arg);
			if (arg[0] && (arg[2] == ':') && (arg[5] == ':')) {
				data = argtou(arg, &arg);
				ds3231[DS3231_REG_HOUR] = tobcd(data);
				data = argtou(arg + 1,&arg);
				ds3231[DS3231_REG_MIN] = tobcd(data);
				data = argtou(arg + 1, &arg);
				ds3231[DS3231_REG_SEC] = tobcd(data);
				ds3231_write(DS3231_REG_SEC, 3);
			} else {
				rtc_print_time();
				uart_putc('\n');
			}
			goto EOK;
		}
		if (str_is(arg, "32k")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				i = 1;
			else if (str_is(arg, "off"))
				i = 0;
			else
				goto EARG;
			ds3231_32k_enable(i);
			goto EOK;
		}
		if (str_is(arg, "sqw")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				ds3231_sqw_enable(true);
			else if (str_is(arg, "off"))
				ds3231_sqw_enable(false);
			else if (str_is(arg, "1hz"))
				ds3231_sqw_freq(DS3231_SQW1HZ);
			else if (str_is(arg, "1k"))
				ds3231_sqw_freq(DS3231_SQW1KHZ);
			else if (str_is(arg, "4k"))
				ds3231_sqw_freq(DS3231_SQW4KHZ);
			else if (str_is(arg, "8k"))
				ds3231_sqw_freq(DS3231_SQW8KHZ);
			else
				goto EARG;
			goto EOK;
		}
	}
	if (str_is(cmd, "timer")) {
		if (*arg == '\0') {
			i = cfg.flags;
			cfg.flags |= CFG_OUT_UART;
			timer();
			cfg.flags = i;
			goto EOK;
		}
		if (str_is(arg, "on")) {
			cfg.flags |= CFG_TIMER_ON;
			goto ESAVE;
		}
		if (str_is(arg, "off")) {
			cfg.flags &= ~CFG_TIMER_ON;
			goto ESAVE;
		}
		if (str_is(arg, "lcd")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				cfg.flags |= CFG_OUT_LCD;
			else if (str_is(arg, "off"))
				cfg.flags &= ~CFG_OUT_LCD;
			else
				goto EARG;
			goto ESAVE;
		}
		if (str_is(arg, "uart")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				cfg.flags |= CFG_OUT_UART;
			else if (str_is(arg, "off"))
				cfg.flags &= ~CFG_OUT_UART;
			else
				goto EARG;
			goto ESAVE;
		}
		goto EARG;
	}
	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
ESAVE:
	cfg_save();
EOK:
	return CLI_EOK;
}

void rtc_print_time(void)
{
	ds3231_read(DS3231_REG_SEC, 3);
	for (uint8_t i = 2;; i--) {
		uart_puth(ds3231[i]);
		if (i)
			uart_putsc(":");
		if (!i)
			break;
	}
	return;
}

void rtc_print_temperature(void)
{
	ds3231_read(DS3231_REG_TEMP_MSB, 2);
	uart_putn(ds3231[DS3231_REG_TEMP_MSB]);
	uart_putsc(".");
	uart_putn((ds3231[DS3231_REG_TEMP_LSB] >> 6) * 25);
	return;
}

void timer(void)
{
	uint8_t dht_err;
	if (cfg.flags & CFG_OUT_UART) {
		rtc_print_time();
		uart_putc(' ');
		rtc_print_temperature();
		dht_err = dht_read();
		if (dht_err == DHT_OK) {
			uart_putsc(" RH: ");
			uart_putn(dht.data.rh / 10);
			uart_putsc(".");
			uart_putn(dht.data.rh % 10);
			uart_putsc(" % ");
			uart_putsc("T: ");
			uart_putn(dht.data.tc / 10);
			uart_putsc(".");
			uart_putn(dht.data.tc % 10);
			uart_putsc(" C");
		}
		uart_putsc("\n");
	} else { /* prepare data for LCD below */
		ds3231_read(DS3231_REG_SEC, 3);
		ds3231_read(DS3231_REG_TEMP_MSB, 2);
		dht_err = dht_read();
	}
#ifdef USE_BV4618_LCD
	if (cfg.flags & CFG_OUT_LCD) {
		bv4618_home();
		bv4618_putsc("RTC ");
		for (uint8_t i = 2;; i--) {
			bv4618_puth(ds3231[i]);
			if (i)
				bv4618_putc(':');
			if (!i)
				break;
		}
		bv4618_putc(' ');
		bv4618_putn(ds3231[DS3231_REG_TEMP_MSB]);
		bv4618_putc('.');
		bv4618_putn((ds3231[DS3231_REG_TEMP_LSB] >> 6) * 25);
		bv4618_putc(223); /* ° degree sign if display's encoding is 'English & Japaneese' */
		bv4618_putc('C');
		bv4618_clright();
		bv4618_line(1);
		if (dht_err == DHT_OK) {
			bv4618_putsc("DHT ");
			bv4618_putn(dht.data.rh / 10);
			bv4618_putsc(".");
			bv4618_putn(dht.data.rh % 10);
			bv4618_putsc(" % ");
			bv4618_putsc("  ");
			bv4618_putn(dht.data.tc / 10);
			bv4618_putsc(".");
			bv4618_putn(dht.data.tc % 10);
			bv4618_putc(223); /* ° degree sign if display's encoding is 'English & Japaneese' */
			bv4618_putc('C');
		}
		bv4618_line(2);
		bv4618_putsc("Vdd: ");
		bv4618_putn(get_vdd(ADC_GET_VDD));
		bv4618_putsc(" mV");
	}
#endif
#ifdef USE_PCF8574_LCD
	if (cfg.flags & CFG_OUT_LCD) {
		pcf_home();
		for (uint8_t i = 2;; i--) {
			pcf_puth(ds3231[i]);
			if (i)
				pcf_putc(':');
			if (!i)
				break;
		}
		pcf_putc(' ');
		pcf_putn(ds3231[DS3231_REG_TEMP_MSB]);
		pcf_putc('.');
		pcf_putn((ds3231[DS3231_REG_TEMP_LSB] >> 6) * 25);
		pcf_putc(223); /* ° degree sign if display's encoding is 'English & Japaneese' */
		pcf_putc('C');
		pcf_clright();
		pcf_line(1); /* pcf_line() and pcf_goto() count from 0 */
		if (dht_err == DHT_OK) {
			pcf_putn(dht.data.rh / 10);
			pcf_putsc(".");
			pcf_putn(dht.data.rh % 10);
			pcf_putsc(" % ");
			pcf_putsc("  ");
			pcf_putn(dht.data.tc / 10);
			pcf_putsc(".");
			pcf_putn(dht.data.tc % 10);
			pcf_putc(223); /* ° degree sign if display's encoding is 'English & Japaneese' */
			pcf_putc('C');
		}
		pcf_line(2);
		pcf_putsc("Vdd: ");
		pcf_putn(get_vdd(ADC_GET_VDD));
		pcf_putsc(" mV");
	}
#endif
	return;
}
