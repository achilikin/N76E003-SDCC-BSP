#include <N76E003.h>
#include <irq.h>
#include <tick.h>
#include <uart.h>
#include <terminal.h>


#include "main.h"

#define APP_VERSION "2105.02"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"reset\n"			  /* SW reset */
	"spi speed 1|2|4|8\n" /* SPI clock speed in MHz */
	"spi cpha [0|1]\n"	  /* set/get SPI clock phase select bit */
	"spi spis [0-3]\n"	  /* set/get SPI Interval selection between adjacent bytes */
	"spi write $val\n"	  /* 16bit value for SPI to send */
	"spi send\n"		  /* send 16 byte sequence 0x00, 0x18, 0x00, ... */
	;

val16_t val;

__idata uint8_t spibuf[16];

int8_t commander(__idata char *cmd)
{
	uint8_t i;
	__idata char *arg = get_arg(cmd);

	if (str_is(cmd, "help")) {
		uart_putsc("VER: ");
		uart_putsc(APP_VERSION);
		uart_putsc(" (");
		uart_putn(sdcc_get_code_size());
		uart_putsc(" bytes)\n");

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
		uart_putsc("\nresetting...\n");
		while (!uart_tx_empty());
		sw_reset();
	}
	if (str_is(cmd, "spi")) {
		if (str_is(arg, "speed")) {
			arg = get_arg(arg);
			val.u16 = argtou(arg, &arg);
			if (val.u16 == 8)
				val.u8low = SPI_CLOCK_8MHZ;
			else if (val.u16 == 4)
				val.u8low = SPI_CLOCK_4MHZ;
			else if (val.u16 == 2)
				val.u8low = SPI_CLOCK_2MHZ;
			else if (val.u16 == 1)
				val.u8low = SPI_CLOCK_1MHZ;
			else
				goto EARG;
			uint8_t spien = SPCR & SPCR_SPIEN;
			SPCR &= ~(SPCR_SPIEN | SPCR_SRR);
			SPCR |= val.u8low;
			SPCR |= spien;
			goto EOK;
		}
		if (str_is(arg, "spis")) {
			arg = get_arg(arg);
			if (*arg == '\0') {
				sfr_page(1);
				val.u8low = SPCR2;
				sfr_page(0);
				uart_puthl(val.u8low);
				goto EOK;
			}
			val.u16 = argtou(arg, &arg);
			if (val.u16 > 3)
				goto EARG;
			sfr_page(1);
			SPCR2 = val.u8low;
			sfr_page(0);
			goto EOK;
		}
		if (str_is(arg, "cpha")) {
			arg = get_arg(arg);
			if (*arg == '\0') {
				uart_putnl(!!(SPCR & SPCR_CPHA));
				goto EOK;
			}
			val.u16 = argtou(arg, &arg);
			if (val.u16 > 1)
				goto EARG;
			if (val.u8low)
				SPCR |= SPCR_CPHA;
			else
				SPCR &= ~SPCR_CPHA;
			goto EOK;
		}
		if (str_is(arg, "write")) {
			arg = get_arg(arg);
			val.u16 = argtou(arg, &arg);
			MARK_ON;
			SPDR = val.u8high;
			#if 1
			while (SPSR & SPSR_TXBUF);
			#else
			while (!(SPSR & SPSR_SPIF));
			#endif
			MARK_OFF;
			SPDR = val.u8low;
			while (!(SPSR & SPSR_SPIF));
			MARK_ON;
			SPSR &= ~SPSR_SPIF;
			MARK_OFF;
			goto EOK;
		}
		if (str_is(arg, "send")) {
			for (i = 0; i < 16; i++)
				spibuf[i] = (i & 0x01) ? 0x18 : 0;
			__idata uint8_t *buf = spibuf;
			for (i = 0; i < 16; i++) {
				SPSR = SPSR_DISMODF;
				SPDR = *buf++;
				#if 1
				while (SPSR & SPSR_TXBUF);
				#else
				while (!(SPSR & SPSR_SPIF));
				#endif
			}
			goto EOK;
		}
		goto EARG;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
EOK:
	return CLI_EOK;
}

void timer(void)
{
	return;
}
