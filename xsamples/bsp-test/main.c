#include <N76E003.h>

#include <iap.h>
#include <i2c.h>
#include <tick.h>
#include <event.h>
/* compile with "-DUSE_UART=1" to select UART1 instead of the default UART0 */
#include <uart.h>
#include <terminal.h>
#include <pinterrupt.h>

#include <dht.h>
#include <bv4618.h>
#include <pcf8574.h>

#include "main.h"
#include "cfg.h"
#include "ps2k.h"

/*
 * N76E0003 MCU pinout for this test sw:
 *
 *                 +------------------+
 *              ---| 1 P0.5    P0.4 20|--- PS/2 CLOCK
 *     UART0 TX ---| 2 P0.6    P0.3 19|--- PS/2 DATA
 *     UART0 RX ---| 3 P0.7    P0.2 18|--- ICPCLK [SCL]
 *          RST ---| 4 P2.0    P0.1 17|---
 *              ---| 5 P3.0    P0.0 16|---
 *              ---| 6 P1.7    P1.0 15|--- MARK
 *          GND ---| 7         P1.1 14|--- CLO
 *  [SDA] ICPDA ---| 8 P1.6    P1.2 13|--- WKT signal (1ms interrupt)
 *          VDD ---| 9         P1.3 12|--- SCL
 *        EPOLL ---|10 P1.5    P1.4 11|--- SDA
 *                 +------------------+
 *
 * N76E0003 dev board pinout:
 *                 G R C D V  + 3 T R G
 *                 N S L A D  5 V X X N
 *                 D T C T D  V 3 0 0 D
 *                 | | | | |  | | | | |
 *                 +------------------+
 *         SDA  ---| P1.4         VDD |--- VDD
 *         SCL  ---| P1.3         GND |--- GND
 *         WCT  ---| P1.2        P1.5 |--- EPOLL
 *         CLO  ---| P1.1        P1.6 |--- ICPDA [SDA]
 *         MARK ---| P1.0        P1.7 |---
 *              ---| P0.0        P3.0 |---
 *              ---| P0.1        P2.0 |--- RST
 * [SCL] ICPCLK ---| P0.2        P0.7 |--- UART0 RX
 *    PS/2 DATA ---| P0.3        P0.6 |--- UART0 TX
 *   PS/2 CLOCK ---| P0.4        P0.5 |--- DHT
 *                 +------------------+
 */
uint16_t last_event;
uint16_t cur_event;
event_t evt;

void main(void)
{
	uint8_t tick = PCON & SET_BIT4; /* store POR flag */

	Set_All_GPIO_Quasi_Mode;

	P12_PushPull_Mode; /* pin 13: will be updated from WKT interrupt */
	P15_PushPull_Mode; /* pin 10: events poll frequency */

	MARK_PIN = 0;
	sfr_page(1);
	/* P1.0 (MARK), P1.1 (CLO), P1.2 (WKT) and P1.5 (Epoll) high speed skew rate */
	P1SR |= SET_BIT5 | SET_BIT2 | SET_BIT1 | SET_BIT0;
	sfr_page(0);

	CKCON |= CKCON_CLOEN; /* System Clock Output Enable, P1.1 (CLO) outputs system clock */

	iap_enable();
	iap_aprom_enable();
	cfg_load();

	uart_init(UART_BR_38400, true);
	if (tick)
		set_rc_trim(cfg.trim);

	tick_init(250); /* generate EVT_TIMER every 250 msec, 4 times per second */
	eni(); /* enable interrupts to start tick timer */

	/**
	 * add a delay to stop cursor blinking on programming
	 * as NuLink resets the chip multiple time
	 * plus keyboard can take a lot of time to boot
	 */
	delay(1500);

	dht_init();
	cli_init(test_cli);

	/* PCF8574 works stable only at 100K */
	i2c_init(I2C_CLOCK_100K, 5, false);

	/* initialize pin interrupt to process PS/2 keyboard */
	pin_irq_init_port(PIN_IRQ_PORT0);
	pin_irq_set_pin(KBD_CLOCK_PIN, PIN_IRQ_EDGE | PIN_IRQ_FALL);
	cli_exec("kbd xf5");   /* stop scancode generation */
	cli_exec("kbd xf0 1"); /* switch to scancode set 1 with single byte release scan for most of the keys */
	cli_exec("kbd xf4");   /* start scancode generation */
	delay(50);
	if (kbd_cmd_pending()) /* if cmd is still pending - keyboard is not connected */
		cfg.flags |= CFG_NO_KBD;
	else
		cfg.flags &= ~CFG_NO_KBD;

	cli_exec("help");
#ifdef USE_BV4618_LCD
	cli_exec("bv init");
#endif
 #ifdef USE_PCF8574_LCD
	cli_exec("pcf init");
#endif
	cli_exec("rtc init\n"); /* '\n' will print new command prompt */

	tick = 0;
	event_flush(); /* clear any events */

	/* events processing loop */
	while (1) {
		EPOLL_PIN ^= 1; /* toggle Fpoll output */
		evt.evt = event_get();

		if (evt.type) {
			if (evt.type == EVT_UART_RX) {
				cli_interact(evt.data);
				continue;
			}
			if (evt.type == EVT_TICK) {
				tick ++;
				/* we have 4 tick events per second */
				/* call timer handler if enabled */
				if ((cfg.flags & CFG_TIMER_ON) && (tick >= 4)) {
					tick = 0;
					timer(); /* print RTC & DHT */
				}
				continue;
			}
			if (evt.type & EVT_KBD_SCAN) {
				kbd_event(evt.type, evt.data);
				continue;
			}
			/* for debugging log unprocessed events to the serial port */
			cur_event = millis();
			uart_putn(cur_event);
			uart_putc(' ');
			uart_putn(cur_event - last_event);
			last_event = cur_event;
			uart_putsc(" Event ");
			uart_puth(evt.evt);
			uart_putc(' ');
			uart_puth(evt.data);
			uart_putc('\n');
		}
	}
}

void set_rc_trim(uint8_t rctrim)
{
	trim = rctrim;
	/* stored at POR
	trim0 = RCTRIM0;
	trim1 = RCTRIM1;
	*/

	uint8_t hircmap0, hircmap1;
	uint16_t trim16;

	hircmap0 = trim0;
	hircmap1 = trim1;
	trim16 = ((hircmap0 << 1) + (hircmap1 & 0x01));
	trim16 = trim16 - trim;
	hircmap1 = trim16 & 0x01;
	hircmap0 = trim16 >> 1;
	ta_enable();
	RCTRIM0 = hircmap0;
	ta_enable();
	RCTRIM1 = hircmap1;
	return;
}
