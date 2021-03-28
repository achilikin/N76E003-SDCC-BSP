#include <N76E003.h>

#include <iap.h>
#include <i2c.h>
#include <key.h>
#include <adc.h>
#include <tick.h>
#include <uart.h>
#include <event.h>
#include <ht1621.h>
#include <lcd_lpwm.h>
#include <pwm_range.h>
#include <terminal.h>

#include "main.h"
#include "cfg.h"

/*
 * define up to 8 key names - one key per bit
 */
#define KEY_SW1 SET_BIT0
#define KEY_SW2 SET_BIT1
#define KEY_SW3 SET_BIT2
#define KEY_SW4 SET_BIT3

/* SW2 not used in this example */
#define KEY_BIT_MASK (KEY_SW1 | KEY_SW2 | KEY_SW3 | KEY_SW4)

static void key_handler(uint8_t evt, uint8_t key);

#ifdef KEY_DEBUG
static void print_duty(uint8_t duty);
#endif

/* populate current state of the keys */
uint8_t key_read(void)
{
	/* we use pull-up keys, so default all to bits to 1 (up) */
	register uint8_t keys = 0xFF;
	if (!PIN_SW1)
		keys &= ~KEY_SW1;
	/* SW2 placeholder
	if (!PIN_SW2)
		keys &= ~KEY_SW2;
	*/
	if (!PIN_SW3)
		keys &= ~KEY_SW3;
	if (!PIN_SW4)
		keys &= ~KEY_SW4;
	return keys;
}

void main(void)
{
	event_t evt;
	Set_All_GPIO_Quasi_Mode;

#if (PWM_CHANNEL == 3)
	P04_PushPull_Mode; /* pin 20: PWM3 output */
#else
	P05_PushPull_Mode; /* pin 1: PWM2 output */
#endif
	lcd_init(0);

	sfr_page(1);
#if (PWM_CHANNEL == 3)
	P0SR |= SET_BIT4; /* P0.4/PWM3 high speed skew rate */
	PIOCON1 |= SET_BIT3; /* P0.4/PWM3 pin functions as PWM3 output */
#else
	P0SR |= SET_BIT5; /* P0.5/PWM2 high speed skew rate */
	PIOCON1 |= SET_BIT2; /* P0.5/PWM2 pin functions as PWM3 output */
#endif
	/* configure Schmitt triggered inputs for keys */
#if (TARGET_BOARD == TARGET_NONAME)
	P1S  |= SET_BIT1 | SET_BIT2 | SET_BIT3 | SET_BIT4;
#else
	P1S  |= SET_BIT1 | SET_BIT2 | SET_BIT3;
	P3S  |= SET_BIT0;
#endif
	sfr_page(0);

	/* enable flash memory access */
	iap_enable();
	iap_aprom_enable();

	/* PWM channel initialization */
	cfg_load();
	pwm_init_timer();
	pwm_channel_init(PWM_CHANNEL, cfg.duty); /* activate channel and set default duty value */
	pwm_set_freq(25, PWM_RANGE_100KHZ); /* will apply deafault duty value */
	pwm_channel_enable(PWM_CHANNEL, cfg.lcd_on);
	pwm_start();

	/* initialize UART and enable RX */
	uart_init(UART_BR_38400, true);
	/* initialize timer with tick interval of 250 msec */
	tick_init(250);
	eni();
	/* add a delay to stop cursor blinking on programming */
	/* as NuLink may reset the chip multiple times */
	delay(500);

	/* measure and display Vdd, use evt as a local variable */
	lcd_set_sign(LCD_SIG_IN, true);
	evt.evt = get_vdd(ADC_GET_VDD);
	lcd_printn(evt.evt, 0, 4);
	lcd_printn(cfg.duty, 4, 4);
	lcd_set_sign(LCD_SIG_DOT0, true);
	lcd_set_sign(LCD_SIG_V, true);
	lcd_set_sign(LCD_SIG_PERCENT, true);
	delay(2500);

	/* clear screen and events */
	lcd_clear();
	uart_putc('\n');
	cli_init(lcd_cli);
	event_flush();

	key_init(KEY_BIT_MASK);

	/* read and process events */
	while(1) {
		evt.evt = event_get();

		if (evt.type) {
			if (evt.type == EVT_ERROR) {
				uart_putsc("Event overflow\n");
				continue;
			}

			if (evt.type == EVT_UART_RX) {
				cli_interact(evt.data);
				continue;
			}

			continue; /* check for more events from interrupts */
		}

		/* after processing interrupt events, check the keypad */
		evt.evt = key_event(key_read());

		if (evt.type == EVT_KEY_NONE)
			continue;
#ifdef KEY_DEBUG
		key_evt_debug(evt.type, evt.data);
#endif
		/* reset if SW1 and SW4 pressed together */
		if (key_is_pressed(KEY_SW1 | KEY_SW4))
			cli_exec("reset");

		/* key events processing */
		key_handler(evt.type, evt.data);
		/*
		PWM can generate changes to configuration
		do not save configuration immediately after EVT_KEY_UP,
		wait KEY_CLEANUP_TIME interval
		in case if values need to be adjusted
		*/
		if (evt.type == EVT_KEY_DONE)
			cfg_save();
	}
}

void set_pwm_duty(uint8_t duty)
{
	if (duty == 0)
		pwm_channel_set_mode(PWM_CHANNEL, PWM_MODE_LOW);
	else if (duty == 100)
		pwm_channel_set_mode(PWM_CHANNEL, PWM_MODE_HIGH);
	else {
		pwm_channel_set_mode(PWM_CHANNEL, PWM_MODE_RUN);
		pwm_channel_set_duty(PWM_CHANNEL, duty);
	}
}

#ifdef KEY_DEBUG
void print_duty(uint8_t duty)
{
	if (duty < 100) {
		lcd_set_digit(5, ' ');
		lcd_set_digit(6, duty / 10);
		lcd_set_digit(7, duty % 10);
	} else {
		lcd_set_digit(5, 1);
		lcd_set_digit(6, 0);
		lcd_set_digit(7, 0);
	}
}
#endif

void key_handler(uint8_t evt, uint8_t key)
{
	/* update duty */
	if (evt == EVT_KEY_PRESS) {
		if (key == KEY_SW4) {
			if (cfg.duty < 100)
				cfg.duty += 1;
		} else if (key == KEY_SW3) {
			if (cfg.duty > 1) /* do not turn off backlight, leave at least 1% */
				cfg.duty -= 1;
		} else if (key == KEY_SW1) {
			cfg.lcd_on ^= 0x01;
		}
		if (key & (KEY_SW3 | KEY_SW4)) {
#ifdef KEY_DEBUG
			print_duty(cfg.duty);
#endif
			set_pwm_duty(cfg.duty);
		}
		return;
	}

	/* in case of de-bouncing evt.data == 0 */
	if (evt == EVT_KEY_UP) {
		if (key) {
			if (key & (KEY_SW3 | KEY_SW4))
				set_pwm_duty(cfg.duty);
			if (key & KEY_SW1) {
				if (cfg.lcd_on)
					cli_exec("lcd on");
				else
					cli_exec("lcd off");
			}
		}
	}
}
