#include <N76E003.h>

#include "dump.h"
#include "uart.h"

#define XBUF_SIZE 16
__xdata uint8_t xbuf[XBUF_SIZE];

void dump_xbuf(uint16_t addr, uint8_t len)
{
	uint8_t i, data;
	uart_puth(HIBYTE(addr));
	uart_puth(LOBYTE(addr));
	uart_putsc(" |");
	for (i = 0; i < len; i++) {
		data = xbuf[i];
		uart_putc(' ');
		uart_puth(data);
	}
	for (; i < 16; i++)
		uart_putsc("   ");

	uart_putsc(" | ");
	for (i = 0; i < len; i++) {
		data = xbuf[i];
		if ((data < 0x20) || (data > 0x7F))
			data = '.';
		uart_putc(data);
	}
	for (; i < 16; i++)
		uart_putc(' ');
	uart_putc('\n');
	return;
}

void dump_set_bufc(__code const uint8_t *addr, uint8_t len)
{
	if (len > 16)
		len = 16;
	for (uint8_t i = 0; i < len; i++)
		xbuf[i] = addr[i];
	return;
}

void dump_set_bufi(__idata const uint8_t *addr, uint8_t len)
{
	if (len > 16)
		len = 16;
	for (uint8_t i = 0; i < len; i++)
		xbuf[i] = addr[i];
	return;
}