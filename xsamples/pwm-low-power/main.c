#include <N76E003.h>

#include <adc.h>
#include <iap.h>
#include <pwm.h>
#include <tick.h>
#include <event.h>
#include <uart.h>
#include <terminal.h>

#include "main.h"

uint8_t state = STATE_LED_OFF;
uint8_t power = 0xFF; /* 0 - 100: manual mode, > 100: auto mode */

#define FSYS_16MHZ 0
#define FSYS_10KHZ 1

/**
 *@brief Set Fsys clock frequency
 *
 * @param fsys - FSYS_16MHZ or FSYS_10KHZ
 */
void fsys_set_clock(uint8_t fsys);


/*
 * N76E0003 small dev board pinout:
 *                         G V R T
 *                         N D X X
 *                         D D 0 0
 *                         | | | |
 *                 +---------------------+
 *  STATE.OUT.1 ---|11 P1.4       P1.5 11|--- (BLINK)
 *  STATE.OUT.0 ---|12 P1.3        VDD 10|--- VDD
 *         PWM0 ---|13 P1.2        VIN 9 |--- VDD (VIN)
 *          CLO ---|14 P1.1        GND 8 |--- GND
 *              ---|15 P1.0       P1.7 7 |---
 *              ---|16 P0.0       P3.0 5 |--- SLEEP_TIMER
 *    SLEEP.OUT ---|17 P0.1       P2.0 2 |--- RST [ICP]
 *    [ICP] CLK ---|18 P0.2       P0.7 3 |--- UART0 RX
 *              ---|19 P0.3       P0.6 2 |--- UART0 TX
 *         MARK ---|20 P0.4       P0.5 1 |---
 *                 +---------------------+
 *                        | | | | |
 *                        G R C D V
 *                        N S L A D
 *                        D T K T D
 */

void main(void)
{
	event_t evt;

	Set_All_GPIO_Quasi_Mode;
	P12 = 0;
	/* connected to SLEEP ON/OFF timer. 1 - sleep, 0 - ON. Used to drive EXT0 */
	P30_Input_Mode;
	sfr_page(1);

	/* set P0.4 (MARK) and P1.1 (CLO) high speed skew rate, falling edge faster ~2 nsec */
	P0SR |= SET_BIT4;
	P1SR |= SET_BIT1;
	sfr_page(0);
	CKCON |= CKCON_CLOEN; /* clock output P1.1 enable  */

	iap_enable(); /* enable IAP to read internal band-gap */
	uart_init(UART_BR_38400, true);

	/* external timer will drive external interrupt 0 */
	IT0 = 1; /* set external interrupt EXT0 to edge triggered mode */
	EX0 = 1; /* enable EXT0 interrupt */

	/* init wake-up timer, do not generate timer events */
	tick_init(0);
	EA = 1; /* enable all interrupts to start tick timer */

	/**
	 * add a delay to stop cursor blinking on programming
	 * as NuLink resets the chip multiple time
	 */
	delay(500);
	clr_BODEN; /* disable BOD circuit to save power in power down mode */
	cli_init(commander);
	cli_exec("help\n"); /* '\n' will put promt sign '>' */
	/* cli_exec("help\n") called adc_get_vdd() and initialized internal iap_bgap value */
	/* so we do not need iap anymore and better disable it to save extra ~1.2 mA in power down mode */
	iap_disable();
	while (!uart_tx_empty()); /* flush UART in case if we are going to power down mode*/
	event_flush(); /* clear any events */

	/** PWM configuration start ***********************************************/
	pwm_clock_fsys(); /* select Fsys as PWM clock source */
	PWM_CLOCK_DIV_8; /* 20kHz with PWM_PERIOD = 100 */
	pwm_clear();

	/* for edge aligned PWM frequency == Fpwm /({PWMPH,PWNPL} + 1) */
	pwm_period_set(PWM_PERIOD - 1);
	pwm_duty_set(LED_PWM_CHANNEL, 0);

	PWM_EDGE_TYPE; /* set pwm generator to edge type */
	PWM_INDEPENDENT_MODE;
	/* configure pins P1.2 as PWM0 */
	PIOCON0 |= PIOCON0_P12_PWM0;
	pwm_load(); /* load PWM values */
	pwm_start(); /* and start PWM */

	if (get_vdd() < 300) {
		set_state(STATE_BATTERY_LOW);
		turn_led_on(50);
		turn_led_off();
		pwm_stop(); /* stop PWM to avoid flickering while measuring Vdd */
		fsys_set_clock(FSYS_10KHZ); /* run al low speed */
		wkt_tick_set_mode(WKT_TICK_MINUTE); /* WKT tick interval ~1 minute */
	} else {
		if (SLEEP_TIMER_PIN == 0)
			set_state(STATE_LED_ON);
		else { /* can happen only in debug mode when SLEEP_TIMER_PIN pin controlled mannually */
			set_state(STATE_LED_OFF);
			pwm_stop();
			fsys_set_clock(FSYS_10KHZ);
			wkt_tick_set_mode(WKT_TICK_MINUTE);
		}
	}

	/* events processing loop */
	while (1) {
		uint16_t vdd = 0;
#if DEBUG
		uint8_t current_state = state;
#endif
		set_state(state); /* to update digital outputs for state monitoring */
		switch (state) {
		case STATE_LED_ON:
			if (SLEEP_TIMER_PIN) {
				turn_led_off();
				pwm_stop();
				set_state(STATE_LED_OFF);
				fsys_set_clock(FSYS_10KHZ);
				wkt_tick_set_mode(WKT_TICK_MINUTE);
				break;
			}
			vdd = get_vdd();
			if (vdd < 300) {
				turn_led_off();
				pwm_stop();
				set_state(STATE_BATTERY_LOW);
				fsys_set_clock(FSYS_10KHZ);
				wkt_tick_set_mode(WKT_TICK_MINUTE);
				break;
			}
			if (power <= 100)
				pwm_duty_set(LED_PWM_CHANNEL, power);
			else
				pwm_duty_set(LED_PWM_CHANNEL, get_pwm_power(vdd));
			pwm_load();
			break;
		case STATE_LED_OFF:
			vdd = get_vdd();
			if (vdd < 300) {
				set_state(STATE_BATTERY_LOW);
				fsys_set_clock(FSYS_10KHZ);
				break;
			}
			if (SLEEP_TIMER_PIN == 0) { /* wake up */
				fsys_set_clock(FSYS_16MHZ); /* run at high speed */
				set_state(STATE_LED_ON);
				wkt_tick_set_mode(WKT_TICK_MSEC); /* WKT tick interval to normal 1msec */
				pwm_start();
				turn_led_on(get_pwm_power(vdd));
			}
			break;
		case STATE_BATTERY_LOW:
			vdd = get_vdd();
			if (vdd >= 310) {
				if (SLEEP_TIMER_PIN) {
					set_state(STATE_LED_OFF);
				} else {
					set_state(STATE_LED_ON);
					fsys_set_clock(FSYS_16MHZ);
					wkt_tick_set_mode(WKT_TICK_MSEC);
					pwm_start();
					turn_led_on(get_pwm_power(vdd));
				}
			}
			break;
		}

#if DEBUG
		P15 = (state == STATE_LED_ON) ? 0 : 1;
		if (state != current_state) {
			uart_putsc("Vdd ");
			if (vdd == 0)
				uart_putsc("---");
			else
				uart_putn(vdd);
			uart_putsc(" SLEEP ");
			uart_putn(SLEEP_TIMER_PIN);
			uart_putsc(" ");
			state_print(current_state);
			uart_putsc(" -> ");
			state_print(state);
			uart_putc('\n');
			while (!uart_tx_empty());
		}
#endif

		for (evt.evt = event_get(); evt.type != EVT_NONE; evt.evt = event_get()) {
			if (evt.type == EVT_UART_RX) {
				cli_interact(evt.data);
				continue;
			}
		}

		if (state != STATE_LED_ON) {
			MARK_ON;
			wait(0, POWER_MODE_DOWN);
			MARK_OFF;
		}
	}
}

/* gradually turn on the LED */
void turn_led_on(uint8_t power)
{
	uint8_t i;
	for (i = 0; i <= power; i++) {
		pwm_duty_set(LED_PWM_CHANNEL, i);
		pwm_load();
		delay(20);
	}

	return;
}

/* gradually turn off the LED */
void turn_led_off(void)
{
	uint8_t i = pwm_duty_get(LED_PWM_CHANNEL);

	for (; i > 0; i--) {
		pwm_duty_set(LED_PWM_CHANNEL, i);
		pwm_load();
		delay(20);
	}

	return;
}

uint16_t get_vdd(void)
{
	return adc_get_vdd(ADC_GET_VDD) / 10;
}

#if LED_POWER_USE_MAP

static uint8_t map(int16_t val, int16_t v_min, int16_t v_max, int16_t pwm_min, int16_t pwm_max)
{
	return (val - v_min) * (pwm_max - pwm_min) / (v_max - v_min) + pwm_min;
}

/*
 ~30mA PWM map
	Vdd		PWM	mA
	300.0	100 20
	320.0	100	30
	340.0	65	30
	360.0	50	30
	380.0	43	30
	400.0	38	30
	420.0	35	30

~ 40mA PWM map
	Vdd		PWM	mA
	300.0	100	20
	320.0	100	30
	340.0	80	40
	360.0	61	40
	380.0	54	40
	400.0	48	40
	420.0	44	40
*/

uint8_t get_pwm_power(uint16_t vdd)
{
	val16_t val;
	val.u16 = vdd;

	/* normalize Vdd to 3.00 to 4.20V range */
	if (val.u16 < 300)
		val.u16 = 300;
	if (val.u16 > 420)
		val.u16 = 420;

	if (val.u16 >= 430) /* Vdd > 4.2V, powered by Nulink? */
		val.u8low = 10;
	else if (val.u16 >= 400)
		val.u8low = map(val.u16, 400, 420, 48, 44);
	else if (val.u16 >= 380)
		val.u8low = map(val.u16, 380, 400, 54, 48);
	else if (val.u16 >= 360)
		val.u8low = map(val.u16, 360, 380, 61, 54);
	else if (val.u16 >= 340)
		val.u8low = map(val.u16, 340, 360, 80, 61);
	else if (val.u16 >= 320)
		val.u8low = map(val.u16, 320, 340, 100, 80);
	else
		val.u8low = 100; /* 100% id Vdd lower than 3.2V */
	return val.u8low;
}

#else

uint8_t get_pwm_power(uint16_t vdd)
{
	val16_t val;
	val.u16 = vdd;

	/* normalize Vdd to 3.00 to 4.20V range */
	/* 3.00 = 100% PWM, 4.20 rounded to 70% PWM */
	if (val.u16 < 300)
		val.u16 = 300;
	if (val.u16 > 420)
		val.u16 = 420;
	/* convert to 0 - 1.20 V range */
	val.u16 = val.u16 - 300;
	/* convert 0-1.20 V range to 0-30% range */
	val.u8low = val.u8low / 2;
	/* calculate PWM value */
	val.u8high = 100 - val.u8low;

	return val.u8high;
}

#endif

void wkt_tick_set_mode(enum wkt_tick_mode_t mode)
{
	cli();
	if (mode == WKT_TICK_MSEC) { /* 1ms clock */
		/* clear pre-scaler, so it is 1:1 or 10kHz */
		WKCON &= ~WKCON_WKPS;
		/* roll over every 10 clocks or roughly 1 millisecond */
		RWK = (uint8_t)(255 - 10 + 1);
	} else if (mode == WKT_TICK_SECOND) { /* 1s clock */
		/* clear pre-scaler */
		WKCON &= ~WKCON_WKPS;
		/* and set it to 6 (1024) */
		WKCON |= 6; /* 9.77 clocks per second */
		/* roll over every 10 clocks or roughly ~1 second */
		RWK = (uint8_t)(255 - 10 + 1);
	} else if (mode == WKT_TICK_MINUTE) { /* ~1 min */
		/* clear pre-scaler */
		WKCON &= ~WKCON_WKPS;
		/* and set it to 7 (2048) */
		WKCON |= 7; /* 4.88 clocks per second */
		/* roll over every 255 clocks or roughly 52 seconds */
		RWK = (uint8_t)(0);
	}
	sti();
	return;
}

void fsys_set_clock(uint8_t fsys)
{
#if DEBUG
	fsys = fsys;
#else
	if (fsys == FSYS_16MHZ) {
		set_HIRCEN; /* enable high-speed internal oscillator */
		while (!(CKSWT & SET_BIT5)); /* wait for oscillator to stabilyze */
		clr_OSC1; /* set CKSWT::OSC to 00 to select 16MHz clock */
		return;
	}
	set_OSC1;   /* set CKSWT::OSC to 10 to select 10kHz clock */
	while (CKEN & SET_BIT0); /* wait for switchover */
	clr_HIRCEN; /* disable high-speed internal oscillator */
#endif
	return;
}

void state_print(uint8_t state)
{
	switch (state) {
	case STATE_LED_OFF:
		uart_putsc("STATE_LED_OFF");
	break;	case STATE_LED_ON:
		uart_putsc("STATE_LED_ON ");
		break;
	case STATE_BATTERY_LOW:
		uart_putsc("STATE_BAT_LOW");
		break;
	}
}

/* EXT0 IRQ to wake up on SLEEP_TIMER expiration */
void ext0_interrupt_handler(void) INTERRUPT(IRQ_EXT0, IRQ_EXT0_REG_BANK)
{
	P01 = 0; /* mirror EXT0 input */
}