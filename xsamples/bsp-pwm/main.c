#include <N76E003.h>

#include <pwm.h>
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
 *         PWM0 ---|13 P1.2        P1.5 10|---
 *         PWM1 ---|14 P1.1        P1.6 8 |--- DAT [ICP]
 *         PWM2 ---|15 P1.0        P1.7 7 |---
 *              ---|16 P0.0        P3.0 5 |---
 *              ---|17 P0.1        P2.0 2 |--- RST [ICP]
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
	sfr_page(1);
	/* set P0.4 (MARK) high speed skew rate, falling edge faster ~2 nsec */
	P0SR |= SET_BIT4;
	PWMINTC = SET_BIT5 | SET_BIT4; /* pwm interrupt at central point */
	sfr_page(0);
	MARK_PIN = 0;

	uart_init(UART_BR_38400, true);

	/* generate EVT_TIMER every 250 msec, 4 times per second */
	tick_init(250);
	EA = 1; /* enable all interrupts to start tick timer */

	/**
	 * add a delay to stop cursor blinking on programming
	 * as NuLink resets the chip multiple time
	 */
	delay(500);
	cli_init(commander);
	cli_exec("help\n");

	event_flush(); /* clear any events */
	tick = 0;
	/** PWM configuration start ***********************************************/
	/* configure pins P1.0-P1.2 as PWM0-PWM2 */
	PIOCON0 |= PIOCON0_PIO02 | PIOCON0_PIO01 | PIOCON0_PIO00;
	PWM_CLOCK_FSYS; /* select Fsys as PWM clock source */
	/* set PWM clock to 1 MHz */
	PWM_CLOCK_DIV_16;
	CLRPWM = 1;
	while (CLRPWM);
	/* set PWM period to 1000 clocks, or 1kHz */
	/* for edge aligned PWM frequency == Fpwm /({PWMPH,PWNPL} + 1) */
	/* for center aligned PWM frequency == Fpwm /(2 * {PWMPH,PWNPL}) */
	PWMPH = HIBYTE(999);
	PWMPL = LOBYTE(999);

	/* default duty cicles for PWM0-3*/
	PWM0H = 0;
	PWM0L = 250;
	PWM1H = HIBYTE(500);
	PWM1L = LOBYTE(500);
	PWM2H = HIBYTE(750);
	PWM2L = LOBYTE(750);

	PWM_CENTER_TYPE; /* set pwm generator to center type */
	set_opmode_phased();
	set_inttype_period();
	PMEN = 0x3F; /* mask off all outputs */
	PMD = 0x00;	 /* set all outputs to 0, interrupt will driver them up */

	sti_pwm(); /* enable PWM interrupt */
	LOAD = 1;
	PWMRUN = 1;

	/** PWM configuration stop ************************************************/
	/* events processing loop */
	while (1) {
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

uint8_t pmd_start = 0x01; /* default start channel 0 */
uint8_t pmd_end = 0x04;	  /* default end channel 3 */
uint8_t phase_shift = 0; /* default shift between channel 0 periods */

static uint8_t pmd = 0x01; /* current PWM Mask Data */
static uint8_t phase_clock;

void pwm_interrupt_handler(void) INTERRUPT(IRQ_PWM, IRQ_PWM_REG_BANK)
{
	MARK_ON;

	if (is_opmode_phased()) { /* 'phase' mode */
		if (phase_clock == 1) {
			PMD = 0; /* turn off current PWD output */
			pmd <<= 1;
			if (pmd > pmd_end)
				pmd = pmd_start;
		}
	}

	if (phase_clock > phase_shift)
		phase_clock = 0;

	PWMF = 0; /* clear interrupt before applying new type */

	if (is_inttype_period()) { /* SW defined 'period' interrupt type */
		ta_enable();
		SFRS = 1;
		PWMINTC ^= SET_BIT4; /* toggle between Central and End point interrupts */
		ta_enable();
		SFRS = 0;
	}

	/**
	 * turn on the next PWM output, instruction above provide
	 / ~5.8 usec dead time 'period' inttype with phase shift 0
	 * or ~3.8 usec for dead time for 'end' or 'center' inttype
	 */
	if (is_opmode_phased()) {
		if (phase_clock == 0)
			PMD = pmd;
	}

	phase_clock++;
	MARK_OFF;
}
