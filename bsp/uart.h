/*
  The MIT License (MIT)

  Simple driver for N76E003 UART

  Default defines (can be changed in Makefile):
	#define USE_UART 0 // select UART port 0 or 1
	#define FOSC_16600 // system clock set to 16.600 MHz
*/
#ifndef N76E003_UART_H
#define N76E003_UART_H

#include <stdint.h>
#include <stdbool.h>

#include "irq.h"

#ifdef __cplusplus
extern "C" {
#endif

enum UART_BR {
	UART_BR_2400,
	UART_BR_4800,
	UART_BR_9600,
	UART_BR_19200,
	UART_BR_38400,
	UART_BR_57600,
	UART_BR_115200
};

/**
  Note: add "CFLAGS += -DUSE_UART=1" to the makefile to select UART 1
*/
#ifndef USE_UART
#define USE_UART 0 /**< 0 for UART0 (default), 1 for UART1 */
#endif

#define UART_ERR_OVERFLOW 0x01
#define UART_DATA_VALID	  0x80

#define UART_BUF_SIZE 0x20 /**< buffer located in xdata, size must be power of 2 */

#if USE_UART == 0
	#define IRQ_UART IRQ_UART0
#else
	#define IRQ_UART IRQ_UART1
#endif

extern uint8_t trim, trim0, trim1;

void uart_interrupt_handler(void) INTERRUPT(IRQ_UART,IRQ_UART_REG_BANK);

/**
 * @param baudrate UART_BR_2400, UART_BR_4800, UART_BR_9600, UART_BR_19200,	UART_BR_38400, UART_BR_57600, UART_BR_115200
 * @param rx_enable enable receive, by default TX only mode
 *
 */
void uart_init(enum UART_BR baudrate, bool rx_enable);

uint16_t uart_getc(void);
bool uart_tx_empty(void);

void uart_putc(uint8_t ch);
#define uart_putln() uart_putc('\n')

/** print uint16_t in dec format */
void uart_putn(uint16_t val);

/** print uint16_t in dec format right alignegd */
void uart_putrn(uint16_t val);

/** print uint8_t in hex format */
void uart_puth(uint8_t val);

/** print uint16_t in hex dormat */
#define uart_puthw(val) do{uart_puth(HIBYTE(val));uart_puth(LOBYTE(val));}while(0)
void uart_putbit(uint8_t val);
void uart_puts(__idata const char *str);
void uart_putsc(__code const char *str);

/** print uint8_t in hex with new line */
#define uart_puthl(val); do{uart_puth(val);uart_putc('\n');}while(0)
/** print uint16_t in hex with new line */
#define uart_puthwl(val) do{uart_puth(HIBYTE(val));uart_puth(LOBYTE(val));uart_putc('\n');}while(0)

/** print uint16_t in dec with new line */
#define uart_putnl(val) do{uart_putn(val);uart_putc('\n');}while(0)

#ifdef __cplusplus
}
#endif
#endif
