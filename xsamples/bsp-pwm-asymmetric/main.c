#include <N76E003.h>

#include <adc.h>
#include <pwm.h>
#include <tick.h>
#include <event.h>
#include <uart.h>
#include <terminal.h>

#include "main.h"

uint8_t adc_duty;
uint8_t pwm_signal_mode = PWM_SIGNAL_INPHASE;

/*
 * N76E0003 dev board pinout:
 *                  G R C D V    + 3 T R G
 *                  N S L A D    5 V X X N
 *                  D T K T D    V 3 0 0 D
 *                  | | | | |    | | | | |
 *                 +----------------------+
 *         OUT1 ---|11 P1.4         VDD 9 |--- VDD
 *         OUT2 ---|12 P1.3         GND 7 |--- GND
 *         PWM0 ---|13 P1.2        P1.5 10|---
 *         PWM1 ---|14 P1.1        P1.6 8 |--- DAT [ICP]
 *         PWM2 ---|15 P1.0        P1.7 7 |---
 *              ---|16 P0.0        P3.0 5 |---
 *              ---|17 P0.1        P2.0 2 |--- RST [ICP]
 *    [ICP] CLK ---|18 P0.2        P0.7 3 |--- UART0 RX
 *         SYNC ---|19 P0.3        P0.6 2 |--- UART0 TX
 *         MARK ---|20 P0.4        P0.5 1 |--- ADC_CHANNEL
 *                 +----------------------+
 */

void main(void)
{
	event_t evt;
	uint8_t tick;

	Set_All_GPIO_Quasi_Mode;
	sfr_page(1);
	/* set P0.4 (MARK) and P0.3 (SYNC) high speed skew rate, falling edge faster ~2 nsec */
	P0SR |= SET_BIT4 | SET_BIT3;
	PWMINTC = PWM_IRQ_RISE;
	sfr_page(0);
	MARK_PIN = 0;

	uart_init(UART_BR_38400, true);

	/* use AIN1 (P3.0) to get duty value */
	adc_enable();
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

	/**
	 * configure pins P1.0-P1.1 as PWM0-PWM2
	 * used for debugging with an oscilloscope
	 * can be omitted
	 */
	PIOCON0 |= PIOCON0_PIO00 | PIOCON0_PIO01 | PIOCON0_PIO02;
	pwm_clock_fsys(); /* select Fsys as PWM clock source */
	/* slowest PWM speed to make interrupt stable */
	PWM_CLOCK_DIV_128;
	pwm_clear();

	/* for edge aligned PWM frequency == Fpwm /({PWMPH,PWNPL} + 1) */
	pwm_period_set(PWM_PERIOD - 1);

	/* default duty cycles for PWM0-2 */
	pwm_duty_set(0, PWM_PERIOD / 4); /* valid 0 - PWM_DUTY_MAX */
	pwm_duty_set(1, PWM_PERIOD - PWM_DUTY_MAX/2); /* valid PWM_DUTY_MAX - PWM_PERIOD */
	pwm_duty_set(2, PWM_DUTY_MAX); /* static centre point */

	PWM_EDGE_TYPE; /* set pwm generator to edge type */
	PWM_INDEPENDENT_MODE;

	PIN_OUT1 = 0;
	PIN_OUT2 = 0;

	sti_pwm(); /* enable PWM interrupt */
	pwm_load(); /* load PWM values */
	pwm_start(); /* and start PWM */

	/* initialize ADC */
	adc_mode_set();
	P05_Input_Mode; /* our ADC_AIN connected to AIN4 */
	AINDIDS |= 1 << ADC_CHANNEL;
	adc_clear();
	adc_select_channel(ADC_CHANNEL);
	adc_start();

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

		if (adc_done()) {
			adc_duty = adc_read() / 40;
			if (adc_mode_get())
				pwm_set_signal_duty(0, adc_duty);
			adc_clear();
			adc_start();
		}
	}
}

void pwm_set_signal_duty(uint8_t channel, uint8_t duty)
{
	if (pwm_signal_mode == PWM_SIGNAL_INPHASE) {
		PWM0L = duty;
		PWM1L = duty + PWM_DUTY_MAX;
	} else if (pwm_signal_mode == PWM_SIGNAL_ANTIPHASE) {
		PWM0L = duty;
		PWM1L = PWM_PERIOD - duty;
	} else {
		if (channel == 0)
			PWM0L = duty;
		else
			PWM1L = duty;
	}
	pwm_load();
}

enum pwm_irq_handler_type_t {
	IRQ_TYPE_START,
	IRQ_TYPE_CH0_FALL,
	IRQ_TYPE_CENTER,
	IRQ_TYPE_CH1_FALL
};

static uint8_t irq_handler_type = IRQ_TYPE_START;

/** quite complex and cumbersome PWM IRQ processing, to be optimized... */
void pwm_interrupt_handler(void) INTERRUPT(IRQ_PWM, IRQ_PWM_REG_BANK)
{
	MARK_ON;
	uint8_t irq = PWMINTC;

	switch (irq_handler_type) {
	case IRQ_TYPE_START:
		SYNC_PULSE; /* sync pulse for oscilloscope */
		PIN_OUT1 = 0;
		PIN_OUT2 = 0;
		/* calculate the next IRQ type depending on CH0 pulse width */
		/* if pulse is too short or too wide - next IRQ type is the center point */
		if ((PWM0L < 2) || (PWM0L > (PWM_PERIOD / 2 - 1))) {
			PIN_OUT1 = (PWM0L > 1) ? 1 : 0;
			irq_handler_type = IRQ_TYPE_CENTER;
			irq = PWM_IRQ_FALL | PWM_IRQ_PWM2;
			break;
		}
		PIN_OUT1 = 1;
		/* next IRQ type - CH0 falling edge */
		irq_handler_type = IRQ_TYPE_CH0_FALL;
		irq = PWM_IRQ_FALL | PWM_IRQ_PWM0;
		break;
	case IRQ_TYPE_CH0_FALL:
		PIN_OUT1 = 0;
		/* next IRQ type - center point */
		irq_handler_type = IRQ_TYPE_CENTER;
		irq = PWM_IRQ_FALL | PWM_IRQ_PWM2;
		break;
	case IRQ_TYPE_CENTER:
		PIN_OUT1 = 0; /* just in case */
		/* if pulse is too short or too wide - next IRQ type is the center point */
		if ((PWM1L < (PWM_PERIOD / 2 + 2)) || (PWM1L > (PWM_PERIOD - 1))) {
			if (PWM1L < (PWM_PERIOD / 2 + 2))
				PIN_OUT2 = 0;
			else
				PIN_OUT2 = 1;
			/* next IRQ type - start point */
			irq_handler_type = IRQ_TYPE_START;
			irq = PWM_IRQ_RISE | PWM_IRQ_PWM2;
			break;
		}
		PIN_OUT2 = 1;
		/* next IRQ type - CH1 falling edge */
		irq_handler_type = IRQ_TYPE_CH1_FALL;
		irq = PWM_IRQ_FALL | PWM_IRQ_PWM1;
		break;
	case IRQ_TYPE_CH1_FALL:
		PIN_OUT2 = 0;
		/* next IRQ type - start point */
		irq_handler_type = IRQ_TYPE_START;
		irq = PWM_IRQ_RISE | PWM_IRQ_PWM2;
		break;
	}

	PWMF = 0; /* clear interrupt before applying new type */
	sfr_page(1);
	PWMINTC = irq;
	sfr_page(0);

	MARK_OFF;
}

void timer(void)
{
	return;
}
