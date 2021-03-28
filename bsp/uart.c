/*
  The MIT License (MIT)

  Simple driver for N76E003 UART

  Default defines (can be changed in Makefile):
	#define USE_UART 0 // select UART port 0 or 1
	#define FOSC_16600 // system clock set to 16.600 MHz
*/
#include <N76E003.h>

#include "uart.h"
#include "event.h"

#define UART_BUF_MASK (UART_BUF_SIZE - 1)

static volatile uint8_t tx_idx;   /* ISR reads from this position */
static volatile uint8_t tx_num;   /* number of bytes in the transmit buffer */
static volatile __bit   tx_empty; /* true if transmit buffer is empty */

static __xdata uint8_t tx_buffer[UART_BUF_SIZE];

#if defined FOSC_16600
static const uint8_t br_reload[7][2] = {
	// RHx  RLx        Value Baudrate Actual       Error %
	{0xFE, 0x50},  // 65104   2400   2401.620370 -0.067
	{0xFF, 0x28},  // 65320   4800   4803.240741 -0.067
	{0xFF, 0x94},  // 65428   9600   9606.481481 -0.067
	{0xFF, 0xCA},  // 65482  19200   19212.96296 -0.067
	{0xFF, 0xE5},  // 65509  38400   38425.92593 -0.067
	{0xFF, 0xEE},  // 65518  57600   57638.88889 -0.067
	{0xFF, 0xF7}   // 65527 115200   115277.7778 -0.067
};
#else
#ifndef FOSC_16000
#define FOSC_16000
#endif
static const uint8_t br_reload[7][2] = {
	// RHx  RLx        Value Baudrate Actual       Error %
	{ 0xFE, 0x5F }, // 65119   2400   2398.081535  0.079
	{ 0xFF, 0x30 }, // 65328   4800   4807.692308 -0.160
	{ 0xFF, 0x98 }, // 65432   9600   9615.384615 -0.160
	{ 0xFF, 0xCC }, // 65484  19200   19230.76923 -0.160
	{ 0xFF, 0xE6 }, // 65510  38400   38461.53846 -0.160
	{ 0xFF, 0xEF }, // 65519  57600   58823.52941 -2.124
	{ 0xFF, 0xF7 }  // 65527 115200   111111.1111  3.549
};
#endif

#if USE_UART == 0
	#define UART_RI  RI
	#define UART_TI  TI
	#define cli_es() clr_ES
	#define sti_es() set_ES
	#define sti_ti() set_TI
#else
	#define UART_RI  RI_1
	#define UART_TI  TI_1
	#define cli_es() clr_ES_1
	#define sti_es() set_ES_1
	#define sti_ti() set_TI_1
#endif

void uart_init(enum UART_BR baudrate, bool rx_enable)
{
#if defined FOSC_16600
	set_fosc_16600(); /* initialize default trim values */
#endif

#if USE_UART == 0
	SCON = 0x40;  	/* UART0 Mode 1 */
	set_SMOD;		/* UART0 Double Rate Enable */
	set_BRCK;		/* Timer 3 as UART0 baud rate clock source */

	P06_Quasi_Mode;	/* UART0 TX pin */

	if (rx_enable) {
		P07_Quasi_Mode;	/* UART0 RX pin */
		set_REN;		/* Receive enabled */
	}
#elif USE_UART == 1
	SCON_1 = 0x40;		/* UART1 Mode 1 */

	P16_Quasi_Mode;		/* UART1 TX pin */
	if (rx_enable) {
		P02_Quasi_Mode;	/* UART1 RX pin */
		set_REN_1;		/* Receive enabled */
	}
#endif
	RH3 = br_reload[baudrate][0];
	RL3 = br_reload[baudrate][1];
	T3CON &= 0xF8;	/* set pre-scale 1/1 */
	tx_empty = 1;
	set_TR3;		/* start Timer 3 */
}

void uart_interrupt_handler(void) INTERRUPT(IRQ_UART,IRQ_UART_REG_BANK)
{
	if (UART_RI) {
		UART_RI = 0;
		event_put(EVT_UART_RX, SBUF);
	}

	if (UART_TI) {
		UART_TI = 0;
		if (tx_num) {
			SBUF = tx_buffer[tx_idx];
			tx_idx += 1;
			tx_idx &= UART_BUF_MASK;
			tx_num -= 1;
		} else
			tx_empty = 1;
	}
}

bool uart_tx_empty(void)
{
	return tx_empty;
}

/** send single char */
void uart_putc(uint8_t ch)
{
	while (tx_num == UART_BUF_SIZE);

	cli_es();
	uint8_t tx_pos = (tx_idx + tx_num) & UART_BUF_MASK;
	tx_buffer[tx_pos] = ch;
	tx_num += 1;
	if (tx_empty) {
		sti_ti();
		tx_empty = 0;
	}
	sti_es();
}

/** send string from __code memory */
void uart_putsc(__code const char *str)
{
	uint8_t ch = *str++;
	while (ch) {
		uart_putc(ch);
		ch = *str++;
	}
}

/** send string from __idata memory */
void uart_puts(__idata const char *str)
{
	uint8_t ch = *str++;
	while (ch) {
		uart_putc(ch);
		ch = *str++;
	}
}

void uart_putbit(uint8_t val)
{
	uint8_t hex = '0';
	if (val & 0x01)
		hex++;
	uart_putc(hex);
}

/** print uint8_t in hex format */
void uart_puth(uint8_t val)
{
	uint8_t hex = val >> 4;
	hex += '0';
	if (hex > '9')
		hex += 7;
	uart_putc(hex);
	hex = val & 0x0F;
	hex += '0';
	if (hex > '9')
		hex += 7;
	uart_putc(hex);
}

/** print uint16_t in dec format left alighted */
void uart_putn(uint16_t val)
{
	if (val == 0) {
		uart_putc('0');
		return;
	}
	uint8_t print = 0;
	uint16_t div = 10000;
	for (uint8_t i = 0; i < 5; i++) {
		uint8_t byte = val / div;
		if (byte)
			print++;
		if (print)
			uart_putc(byte + '0');
		val -= byte * div;
		div /= 10;
	}
	return;
}

static __xdata uint8_t nbuf[6];

/** print uint16_t in dec format right alignegd */
void uart_putrn(uint16_t val)
{
	uint8_t print;
	for (print = 0; print < 5; print++)
		nbuf[print] = ' ';

	nbuf[print] = '\0';
	if (val == 0)
		nbuf[4] = '0';
	else {
		uint16_t div = 10000;
		print = 0;
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t byte = val / div;
			if (byte)
				print++;
			if (print)
				nbuf[i] = byte + '0';
			val -= byte * div;
			div /= 10;
		}
	}

	for (print = 0; print < 5; print++)
		uart_putc(nbuf[print]);
	return;
}
