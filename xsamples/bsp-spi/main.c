#include <N76E003.h>

#include <tick.h>
#include <event.h>
#include <uart.h>
#include <terminal.h>


#include "main.h"

/*
  * N76E0003 dev board pinout:
 *                  G R C D V    + 3 T R G
 *                  N S L A D    5 V X X N
 *                  D T K T D    V 3 0 0 D
 *                  | | | | |    | | | | |
 *                 +----------------------+
 *              ---|11 P1.4         VDD 9 |--- VDD
 *              ---|12 P1.3         GND 7 |--- GND
 *              ---|13 P1.2        P1.5 10|--- SS
 *              ---|14 P1.1        P1.6 8 |--- DAT [ICP]
 *        SPCLK ---|15 P1.0        P1.7 7 |---
 *         MOSI ---|16 P0.0        P3.0 5 |---
 *         MISO ---|17 P0.1        P2.0 2 |--- RST [ICP]
 *    [ICP] CLK ---|18 P0.2        P0.7 3 |--- UART0 RX
 *              ---|19 P0.3        P0.6 2 |--- UART0 TX
 *         MARK ---|20 P0.4        P0.5 1 |---
 *                 +----------------------+
 */

void main(void)
{
	event_t evt;
	uint8_t tick;

	Set_All_GPIO_Quasi_Mode;
	P00_PushPull_Mode;
	MARK_PIN = 0;

	sfr_page(1);
	/* set high speed skew rate, falling edge faster ~2 nsec */
	P0SR |= SET_BIT0 | SET_BIT4; /* P0.0 (MOSI), P0.4 (MARK) */
	P1SR |= SET_BIT0; /* SPCLK */
	sfr_page(0);

	SPSR |= SPSR_DISMODF; /* disable mode fault error flag */
	/**
	 * SPI in master mode
	 * SS as general I/O
	 * SPCLK to 0 in idle mode
	 * MSB first
	 */
	SPCR = SPCR_MSTR | SPI_CLOCK_8MHZ | SPCR_SPIEN;
	SPSR &= ~SPSR_SPIF; /* clear SPI complete flag */

	uart_init(UART_BR_38400, true);
	/**
	 * generate EVT_TIMER every 250 msec, 4 times per second
	 * 253 instead of 250 is used because we are running at 16.6MHz
	 */
	tick_init(253);
	EA = 1; /* enable all interrupts to start tick timer */

	/**
	 * add a delay to stop cursor blinking on programming
	 * as NuLink resets the chip multiple time
	 */
	delay(500);

	cli_init(commander);
	cli_exec("help\n"); /* '\n' will put promt sign '>' */

	event_flush(); /* clear any events */
	tick = 0;

	/* events processing loop */
	while (1) {
		MARK; /* pulse MARK_PIN to measure poll cycle */
		evt.evt = event_get();

		if (evt.type) {
			if (evt.type == EVT_UART_RX) {
				cli_interact(evt.data);
				continue;
			}
			if (evt.type == EVT_TICK) {
				tick++;
				/* we have 4 tick events per second, so count to 4 before calling our timer handler */
				if (tick >= 4) {
					timer();
					tick = 0;
				}
				continue;
			}
		}
	}
}
