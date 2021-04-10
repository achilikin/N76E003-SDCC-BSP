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
#include <terminal.h>

#include "main.h"
#include "cfg.h"
#include "pwm_range.h"

/*
 * define up to 8 key names - one key per bit
 */
#define KEY_FREQ_INC SET_BIT0
#define KEY_FREQ_DEC SET_BIT1
#define KEY_DUTY_INC SET_BIT2
#define KEY_DUTY_DEC SET_BIT3

#define KEY_BIT_MASK (KEY_FREQ_DEC | KEY_FREQ_INC | KEY_DUTY_DEC | KEY_DUTY_INC)

static uint8_t lcd_update = 1;
static void key_pwm(uint8_t evt, uint8_t key);

static void freq_increment(uint8_t rate);
static void freq_decrement(uint8_t rate);

/* populate current state of the keys */
uint8_t key_read(void)
{
	/* we use pull-up keys, so default all to bits to 1 (up) */
	uint8_t keys = 0xFF;
#if (SWAP_KEYS == 0) /* use default key names: '+' on the left and '-' on the right */
	if (!PIN_FREQ_INC)
		keys &= ~KEY_FREQ_INC;
	if (!PIN_FREQ_DEC)
		keys &= ~KEY_FREQ_DEC;
	if (!PIN_DUTY_INC)
		keys &= ~KEY_DUTY_INC;
	if (!PIN_DUTY_DEC)
		keys &= ~KEY_DUTY_DEC;
#else /* swap '+/-' position, so '-' is on the left and '+' on the right */
	if (!PIN_FREQ_DEC)
		keys &= ~KEY_FREQ_INC;
	if (!PIN_FREQ_INC)
		keys &= ~KEY_FREQ_DEC;
	if (!PIN_DUTY_DEC)
		keys &= ~KEY_DUTY_INC;
	if (!PIN_DUTY_INC)
		keys &= ~KEY_DUTY_DEC;
#endif
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
	pwm_channel_init(PWM_CHANNEL, cfg.duty);
	if (!(cfg.flags & CGF_PWM_DIRECT))
		pwm_channel_set_polarity(PWM_CHANNEL, PWM_POLARITY_NEGATIVE);
	pwm_set_freq(cfg.freq, cfg.range);

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
	lcd_set_sign(LCD_SIG_DOT0, true);
	lcd_set_sign(LCD_SIG_V, true);
	delay(2500);

	/* clear screen and events */
	lcd_init(0x00);
	uart_putc('\n');
	cli_init(lpwm_cli);
	event_flush();

	key_init(KEY_BIT_MASK);
	if (cfg.flags & CGF_PWM_RUN)
		cli_exec("out on");
	else
		cli_exec("out off");

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
		/* reset if DUTY+ and FREQ+ pressed together */
		if (key_is_pressed(KEY_FREQ_DEC | KEY_DUTY_INC))
			cli_exec("reset");

		/* key events processing */
		key_pwm(evt.type, evt.data);
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

void freq_increment(uint8_t rate)
{
	cfg.freq += rate;

	if (cfg.range == PWM_RANGE_100KHZ) {
		if (cfg.freq > 160)
			cfg.freq = 160;
		return;
	}

	if ((cfg.range == PWM_RANGE_1HZ) && cfg.freq > 99) {
		cfg.range = PWM_RANGE_100HZ;
		return;
	}

	if (cfg.freq > 999) {
		cfg.freq = 100 + cfg.freq - 1000;
		cfg.range += 1;
		return;
	}
}

void freq_decrement(uint8_t rate)
{
	if (cfg.range == PWM_RANGE_1HZ) {
		if (cfg.freq <= rate)
			rate = 1;
		if (cfg.freq > rate)
			cfg.freq -= rate;
		return;
	}

	cfg.freq -= rate;

	if ((cfg.range == PWM_RANGE_100HZ) && cfg.freq < 100) {
		cfg.range = PWM_RANGE_1HZ;
		return;
	}

	if (cfg.freq < 100) {
		cfg.freq = 1000 - (100 - cfg.freq);
		cfg.range -= 1;
		return;
	}
}

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

void print_range(uint8_t range)
{
	lcd_set_sign(LCD_SIG_DOT0, range <= PWM_RANGE_100HZ);
	lcd_set_sign(LCD_SIG_DOT1, range == PWM_RANGE_1KHZ);
	lcd_set_sign(LCD_SIG_DOT2, range == PWM_RANGE_10KHZ);
}

void print_off(uint8_t dstart)
{
	lcd_set_digit(dstart++, ' ');
	lcd_set_digit(dstart++, 0);
	lcd_set_digit(dstart++, 0x0F);
	lcd_set_digit(dstart++, 0x0f);
}

void set_pwm_duty(uint8_t duty)
{
	if (duty == 0)
		pwm_channel_set_mode(PWM_CHANNEL, PWM_MODE_LOW);
	else if (duty == 100)
		pwm_channel_set_mode(PWM_CHANNEL, PWM_MODE_HIGH);
	else
		pwm_channel_set_mode(PWM_CHANNEL, PWM_MODE_RUN);
}

/**
 * converts string to PWM value
 * @param str 4 chars, 'k' specifying kHz range
 *   kxxx: 1 to 999 Hz
 *   xkxx: 1 to 9.99 kHz
 *   xxkx: 10 to 99.9 kHz
 *   xxxk: 100 to 160 kHz
 */
uint16_t pwm_str2freq(__idata char *str)
{
	uint8_t i, idx, pos;
	__idata char buf[4];

	for(i = idx = pos = 0; i < 4; i++) {
		if (str[i] == 'k') {
			pos = i;
			continue;
		}
		if ((str[i] < '0') || (str[i] > '9'))
			break;
		buf[idx] = str[i];
		idx++;
	}
	if (idx != 3)
		return 0;
	buf[idx] = '\0';

	uint16_t freq = argtou(buf, &str);
	uint8_t range = PWM_RANGE_1HZ;

	if (freq == 0)
		freq = 1;

	if (pos > 0)
		range = PWM_RANGE_100HZ + pos;
	if ((range == PWM_RANGE_1HZ) && (freq > 99))
		range = PWM_RANGE_100HZ;
	if ((range == PWM_RANGE_100KHZ) && (freq > 160))
		freq = 160;

	freq |= range << 12;

	return freq;
}

void key_pwm(uint8_t evt, uint8_t key)
{
	/* increment frequency update rate after very long press */
	if (evt == EVT_KEY_REPEAT) {
		if (key == 4) /* repeat event counter */
			lcd_update = 10;
		return;
	}

	/* update frequency/duty */
	if (evt == EVT_KEY_PRESS) {
		lcd_set_sign(LCD_SIG_SET, true);
		lcd_set_sign(LCD_SIG_OUT, false);

		if (key == KEY_FREQ_INC)
			freq_increment(lcd_update);
		else if (key == KEY_FREQ_DEC)
			freq_decrement(lcd_update);
		else if (key == KEY_DUTY_INC) {
			if (cfg.duty < 100)
				cfg.duty += 1;
		} else if (key == KEY_DUTY_DEC) {
			if (cfg.duty > 0)
				cfg.duty -= 1;
		}

		if (key & (KEY_DUTY_INC | KEY_DUTY_DEC))
			print_duty(cfg.duty);
		else {
			print_freq(cfg.freq);
			print_range(cfg.range);
		}
		return;
	}

	/* in case of de-bouncing evt.data == 0 */
	if (evt == EVT_KEY_UP) {
		if (key) {
			if (key & (KEY_DUTY_INC | KEY_DUTY_DEC))
				set_pwm_duty(cfg.duty);
			else
				pwm_set_freq(cfg.freq, cfg.range);

			lcd_update = 1;
			lcd_set_sign(LCD_SIG_SET, false);
			lcd_set_sign(LCD_SIG_OUT, true);
		}
	}
}
