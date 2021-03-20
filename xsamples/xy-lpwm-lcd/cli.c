#include <N76E003.h>

#include <iap.h>
#include <irq.h>
#include <pwm.h>
#include <adc.h>
#include <dump.h>
#include <uart.h>
#include <ht1621.h>
#include <lcd_lpwm.h>
#include <terminal.h>

#include "main.h"
#include "cfg.h"

#define APP_VERSION "2007.28"

#if (TARGET_BOARD == VHM_800_BOARD)
	#define BOARD_NAME "VHM-800"
#elif (TARGET_BOARD == NONAME_BOARD)
	#define BOARD_NAME "NONAME"
#else
	#define BOARD_NAME "XY-LPWM"
#endif

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"reset\n"					  /* software reset example */
	"lcd 0#...7#\n"				  /* display symbol per position */
	"lcd @0-8#...#\n"			  /* display string of symbols starting at specified position */
	"lcd raw 0-7 xXX\n"			  /* write to lcd off-screen memory */
	"lcd map 0-17 xX\n"			  /* write raw value directly to HT1621 segment map memory  */
	"lcd sig S#...S#\n"			  /* control signs */
	"lcd led [1-100]\n"			  /* set backlight level */
	"lcd on|off|clear|fill|dump"; /* lcd control commands */

/* cli parser */
int8_t lcd_cli(__idata char *cmd)
{
	uint8_t i;
	__idata char *arg = get_arg(cmd);
	/* print some basic information about the app and the chip */
	if (str_is(cmd, "help")) {
		uart_putsc("VER: ");
		uart_putsc(BOARD_NAME);
		uart_putc(' ');
		uart_putsc(APP_VERSION);
		uart_putsc(" (");
		uart_putn(sdcc_get_code_size());
		uart_putsc(" bytes)\n");
		uart_putsc("BGP: ");
		uint16_t bgap = get_vdd(ADC_GET_RAW_BGAP);
		uart_puthw(bgap);
		uart_putc(' ');
		bgap = get_vdd(ADC_GET_BGAP);
		uart_putn(bgap);
		uart_putsc("mV\nVDD: ");

		bgap = get_vdd(ADC_GET_VDD);
		uart_putn(bgap);
		uart_putsc("mV\nUID:");

		for (i = 1; i <= IAP_UID_SIZE; i++) {
			uart_putc(' ');
			uart_puth(iap_read_uid(IAP_UID_SIZE-i));
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

	/* SW reset */
	if (str_is(cmd, "reset")) {
		uart_putsc("resetting...");
		while (!uart_tx_empty());
		sw_reset();
	}

	if (str_is(cmd, "lcd")) {
		/**
		 * addressing digits individually
		 * display up to 8 symbols in the 'IS' format,
		 * where 'I' is the digit Index and 'S' the symbol to display,
		 * for example, 'lcd 0A2B3C4D' will display 'ABCd',
		 * 'lcd 0A1 2 3A' will display 'A  A'
		 */
		if ((*arg >= '0') && (*arg <= '7')) {
			for (i = 0; i < (LCD_NUM_DIGITS * 2); i++) {
				if (arg[i] == 0)
					break;
				uint8_t n = arg[i++] - '0'; /* digit index */
				lcd_set_symbol(n, arg[i]);
			}
			goto EOK;
		}
		/**
		 * addressing digits sequentially
		 * display up to 8 symbols with dots starting from the given digit index,
		 * for example, 'lcd @0Food' will display 'Food' in the first line,
		 * 'lcd @11.234567.0' will display
		 *     '01.23' in the first line and
		 *     '45:67' in the second
		 * Note that '.' for position 3 will display 'V' sign
		 * and '.' for position 7 will display colon sign for the line 2
		 */
		if (*arg == '@') {
			arg++;
			if ((*arg < '0') || (*arg > '7'))
				goto EARG;
			i = *arg - '0'; /* start digit index */
			arg++;
			while(*arg) {
				lcd_set_symbol(i, *arg++);
				if (*arg == '.') {
					lcd_set_dot(i, true);
					arg++;
				} else
					lcd_set_dot(i, false);
				i = (i + 1) & 0x07;
			}
			goto EOK;
		}
		/**
		 * display up to 8 signs in the 'IS' format,
		 * where 'I' is the sign Index and 'S' value to set: ' ' to turn off, anything else to turn on,
		 * for example, 'I.O.S.C.V.W.%.A.h.' will display all side signs,
		 * '0.1.2.4.5.6.' will display all dots,
		 * ':.' as the first commans with the next ': ' will pulse the colon sign
		 */
		if (str_is(arg, "sig")) {
			arg = get_arg(arg);
			for (i = 0; arg[i]; i++)
				lcd_set_sign(arg[i++], arg[i] > ' ');
			goto EOK;
		}
		/**
		 * write raw value to lcd buffer:
		 * 'lcd raw I xXX' where 'I' is the digit Index and 'xXX' hexadecimal value to write
		 * for example, 'lcd raw 2 x80' will display dot for digit position 2
		 */
		if (str_is(arg, "raw")) {
			arg = get_arg(arg);
			if ((*arg >= '0') && (*arg <= '7')) {
				i = argtou(arg, &arg); /* digit index */
				uint8_t val = argtou(arg, &arg); /* byte in hexadecimal format */
				lcd_set_raw(i, val);
				goto EOK;
			}
		}
		/**
		 * write four bits directly to HT1621 segments map:
		 * 'lcd map I xX' where 'I' is the segment map Index and 'xX' hexadecimal value to write
		 * for example, 'lcd map 16 xF' will display 'C' for digit position 0
		 * See lcd_lpwm.h for HT1621 segments map
		 */

		if (str_is(arg, "map")) {
			arg = get_arg(arg); /* start segment index */
			i = argtou(arg, &arg); /* value to write */
			if (i < LCD_NUM_SEGMENTS) {
				ht1621_write_seg(i, argtou(arg, &arg));
				goto EOK;
			}
		}
		/** clear lcd buffer and screen */
		if (str_is(arg, "clear")) {
			lcd_clear();
			goto EOK;
		}
		/** display all lcd segments */
		if (str_is(arg, "fill")) {
			lcd_fill();
			goto EOK;
		}
		/** dump lcd buffer to serial port */
		if (str_is(arg, "dump")) {
			lcd_dump();
			goto EOK;
		}
		/** turn on backlight led and HT1621 */
		if (str_is(arg, "on")) {
			ht1621_write_cmd(HT1621_SYS_EN);
			ht1621_write_cmd(HT1621_LCD_ON);
			set_pwm_duty(cfg.duty);
			cfg.lcd_on = 1;
			goto SAVE;
		}
		/** turn off backlight led and HT1621 */
		if (str_is(arg, "off")) {
			ht1621_write_cmd(HT1621_SYS_DIS); /* will disable lcd as well */
			set_pwm_duty(0);
			cfg.lcd_on = 0;
			goto SAVE;
		}
		/** control lcd display backlight */
		if ((*arg == '\0') || (str_is(arg, "led"))) {
			arg = get_arg(arg);
			if (*arg) {
				uint8_t duty = argtou(arg, &arg);
				if (!duty || (duty > 100))
					goto EARG;
				set_pwm_duty(duty);
				cfg.duty = duty;
				goto SAVE;
			}

			if (cfg.lcd_on)
				uart_putsc("on");
			else
				uart_putsc("off");
			uart_putsc(" backlight ");
			uart_putn(cfg.duty);
			uart_putsc("%");
			uart_putc('\n');
			goto EOK;
		}
		goto EARG;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
SAVE:
	cfg_save();
EOK:
	return CLI_EOK;
}
