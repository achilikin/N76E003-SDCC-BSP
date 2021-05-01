#include <N76E003.h>
#include <i2c.h>
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
 *         SDA  ---|11 P1.4         VDD  9|--- VDD
 *         SCL  ---|12 P1.3         GND  7|--- GND
 *              ---|13 P1.2        P1.5 10|---
 *              ---|14 P1.1        P1.6 8 |--- DAT [ICP]
 *         MARK ---|15 P1.0        P1.7 7 |---
 *              ---|16 P0.0        P3.0 5 |---
 *              ---|17 P0.1        P2.0 2 |--- RST [ICP]
 *    [ICP] CLK ---|18 P0.2        P0.7 3 |--- UART0 RX
 *              ---|19 P0.3        P0.6 2 |--- UART0 TX
 *              ---|20 P0.4        P0.5 1 |---
 *                 +----------------------+
 *
 *  HPDL-1414 to MCP23017 wiring:
 *  +------+-----+
 * 	| HPDL | MPC |
 *  +------+-----+
 *  | D0   | PA0 |
 *  | D1   | PA1 |
 *  | D2   | PA2 |
 *  | D3   | PA3 |
 *  | D4   | PA4 |
 *  | D5   | PA5 |
 *  | D6   | PA6 |
 *  | A0   | PB0 |
 *  | A1   | PB1 |
 *  | WR   | PB2 |
 *  +------+-----+
 */

uint8_t tick;

void main(void)
{
	event_t evt;

	Set_All_GPIO_Quasi_Mode;
	MARK_PIN = 0;

	sfr_page(1);
	/* set P1.0 (MARK) high speed skew rate, falling edge faster ~2 nsec */
	P1SR |= SET_BIT0;
	sfr_page(0);

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
	/* MCP23017 works fine with up to 1MHz */
	i2c_init(I2C_CLOCK_1MHZ, 5, false);

	cli_init(commander);
	cli_exec("i2c wr x20 0 0 0"); /* set MCP23017 pins to output */
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

void timer_reset(void)
{
	tick = 0;
}