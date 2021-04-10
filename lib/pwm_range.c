/*
  The MIT License (MIT)

  Simple routines to manipulate N76E003 PWM configuration.
  Not all available PWM features are enabled.
*/
#include <N76E003.h>

#include <stdint.h>

#include <irq.h>

#include "pwm_range.h"
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

	CKCON &= ~CKCON_T1M; /* clear T1M to run TIM1 in compatibility mode: Fsys/12 */
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

/** sets duty in percents */
void pwm_channel_set_duty(uint8_t channel, uint8_t duty)
{
	uint16_t duty_counter = (uint16_t)(duty * duty_k + 0.5);
	if (PWMRUN)
		while (LOAD); /* make sure that PWMCON0::LOAD is 0 */

	pwm_duty_set(channel, duty_counter);
	pwm_duty[channel] = duty;
	LOAD = 1;

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
		CKCON |= CKCON_PWMCKS; /* set PWM source TIM1 */
		clr_PWMDIV1; /* set pwm pre-scaler to 0 */
	} else {
		CKCON &= ~CKCON_PWMCKS; /* set pwm source to Fsys */
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
