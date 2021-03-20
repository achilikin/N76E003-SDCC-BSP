/*
  The MIT License (MIT)

  Simple routines to manipulate N76E003 PWM configuration.
  Not all available PWM features are enabled.
*/
#include <N76E003.h>

#include <stdint.h>

#include <irq.h>

#include "pwm.h"
#include "terminal.h"

static __xdata float duty_k;
static __xdata uint8_t pwm_channels; /** mask of active PWM channels */
static __xdata uint8_t pwm_duty[PWM_NUM_CHANNELS];  /** current PWM duty per channel */

#define PWM_TIM1_RELOAD 22

/**
 * will use TIM1 as PWM clock source
 * @param channel channel to activate
 * @param duty default duty value
 */
void pwm_init_timer(void)
{
	pwm_channels = 0;
	for (uint8_t i = 0; i < PWM_NUM_CHANNELS; i++)
		pwm_duty[i] = 0;

	CKCON |= SET_BIT3; /* run TIM1 in compatibility mode */
	/* TIM1 should be set to reload mode for PWM_RANGE_1HZ */
	TMOD &= 0x0F; /* clear TIM1 config */
	TMOD |= 0x20; /* set mode 2 - auto-reload from TH1 */
	TH1 = (uint8_t)(256 - PWM_TIM1_RELOAD);
	TL1 = (uint8_t)(256 - PWM_TIM1_RELOAD);
	TR1 = 1; /* start TIM1 */
}

void pwm_channel_init(uint8_t channel, uint8_t duty)
{
	if (channel < PWM_NUM_CHANNELS) {
		pwm_channels |= 0x01 << channel;
		pwm_duty[channel] = duty;
	}
}

void pwm_channel_set_duty(uint8_t channel, uint8_t duty)
{
	uint16_t duty_counter = (uint16_t)(duty * duty_k + 0.5);
	while (LOAD); /* make sure that PWMCON0::LOAD is 0 */

	switch (channel) {
	case 0:
		PWM0H = HIBYTE(duty_counter);
		PWM0L = LOBYTE(duty_counter);
		break;
	case 1:
		PWM1H = HIBYTE(duty_counter);
		PWM1L = LOBYTE(duty_counter);
		break;
	case 2:
		PWM2H = HIBYTE(duty_counter);
		PWM2L = LOBYTE(duty_counter);
		break;
	case 3:
		PWM3H = HIBYTE(duty_counter);
		PWM3L = LOBYTE(duty_counter);
		break;
	case 4:
		sfr_page(1);
		PWM4H = HIBYTE(duty_counter);
		PWM4L = LOBYTE(duty_counter);
		sfr_page(0);
		break;
	case 5:
		sfr_page(1);
		PWM5H = HIBYTE(duty_counter);
		PWM5L = LOBYTE(duty_counter);
		sfr_page(0);
		break;
	}
	LOAD = 1;
	pwm_duty[channel] = duty;
	return;
}

/**
 * @param pwm_freq 3 digit value for selected range, for example:
 * 		777 for PWM_RANGE_100HZ means 777. Hz
 * 		777 for PWM_RANGE_10KHZ means 7.77 kHz
 * @param range one of PWM_RANGE_* values
 */
void pwm_set_freq(uint16_t pwm_freq, uint8_t range)
{
	uint32_t fpwm = HIRC_FREQ;
	static const float freq_k[PWM_RANGE_NUM] = { 1, 1, 1, 10, 100, 1000 };

	/* convert frequency to counters */
	if (range == PWM_RANGE_1HZ) {
		fpwm /= 12 * 22; // 62878L;
		set_PWMCKS; /* set PWM source TIM1 */
		clr_PWMDIV1; /* set pwm pre-scaler to 0 */
	} else {
		clr_PWMCKS;  /* set pwm source to Fsys */
		if (range <= PWM_RANGE_1KHZ) {
			fpwm /= 4;
			set_PWMDIV1; /* set pwm pre-scaler to 4 */
		} else
			clr_PWMDIV1; /* set pwm pre-scaler to 0 */
	}

	/* PWM frequency = Fpwm/((PWMPH,PWMPL) + 1) */
	/* PWM divider = Fpwm/(PWM frequency) - 1 */
	float fdiv = (float)fpwm / ((float)pwm_freq * freq_k[range]);
	if (range > PWM_RANGE_1KHZ)
		fdiv += 0.5;

	uint16_t pwm_div = (uint16_t)fdiv;
	duty_k = pwm_div / 100.0;

	while (LOAD); /* make sure that PWMCON0::LOAD is 0 */
	PWMPH = HIBYTE(pwm_div);
	PWMPL = LOBYTE(pwm_div);
	/* recalculate new duty configuration */
	for (uint8_t i = 0; i < 6; i++) {
		if (pwm_channels & (0x01 << i))
			pwm_channel_set_duty(i, pwm_duty[i]);
	}
}

/** controls PNP (PWM Negative Polarity) register */
void pwm_channel_set_polarity(uint8_t channel, uint8_t negative)
{
	uint8_t mask = 1 << channel;
	if (negative)
		PNP |= mask;
	else
		PNP &= ~mask;
	return;
}

/** reads PNP (PWM Negative Polarity) register for given channel */
uint8_t pwm_channel_get_polarity(uint8_t channel)
{
	uint8_t mask = 1 << channel;
	return (PNP & mask);
}

/**
 * controls PMEN (PWM Mask Enable) register:
 * @param channel channel index to control
 * @param enable true to generate PWM, false to set static level controlled by PMD register
 */
void pwm_channel_enable(uint8_t channel, bool enable)
{
	uint8_t mask = 1 << channel;
	if (enable)
		PMEN &= ~mask;
	else
		PMEN |= mask;
	return;
}

uint8_t pwm_channel_is_enabled(uint8_t channel)
{
	uint8_t mask = 1 << channel;
	return !(PMEN & mask);
}

/**
 * controls PMD (PWM Mask Data) register if enabled by PMEN:
 *  0: signal level is set to 0
 *  1: signal level is set to 1
*/
void pwm_channel_set_level(uint8_t channel, uint8_t set)
{
	uint8_t mask = 1 << channel;
	if (set)
		PMD |= mask;
	else
		PMD &= ~mask;
	return;
}

void pwm_channel_set_mode(uint8_t channel, enum pwm_mode_t mode)
{
	if (mode == PWM_MODE_RUN) {
		pwm_channel_enable(channel, true);
		return;
	}

	pwm_channel_enable(channel, false);
	pwm_channel_set_level(channel, mode == PWM_MODE_HIGH);
	return;
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

