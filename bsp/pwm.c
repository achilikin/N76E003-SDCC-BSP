/*
  The MIT License (MIT)

  Simple routines to manipulate N76E003 PWM configuration.
  Not all available PWM features are enabled.
*/
#include "pwm.h"

void pwm_duty_set(uint8_t channel, uint16_t duty)
{
	switch(channel) {
	case 0:
		PWM0L = LOBYTE(duty);
		PWM0H = HIBYTE(duty);
		break;
	case 1:
		PWM1L = LOBYTE(duty);
		PWM1H = HIBYTE(duty);
		break;
	case 2:
		PWM2L = LOBYTE(duty);
		PWM2H = HIBYTE(duty);
		break;
	case 3:
		PWM3L = LOBYTE(duty);
		PWM3H = HIBYTE(duty);
		break;
	case 4:
		sfr_page(1);
		PWM4L = LOBYTE(duty);
		PWM4H = HIBYTE(duty);
		sfr_page(0);
		break;
	case 5:
		sfr_page(1);
		PWM5L = LOBYTE(duty);
		PWM5H = HIBYTE(duty);
		sfr_page(0);
		break;
	}
	return;
}

uint16_t pwm_duty_get(uint8_t channel)
{
	val16_t duty;

	switch(channel) {
	case 0:
		duty.u8low = PWM0L;
		duty.u8high = PWM0H;
		break;
	case 1:
		duty.u8low = PWM1L;
		duty.u8high = PWM1H;
		break;
	case 2:
		duty.u8low = PWM2L;
		duty.u8high = PWM2H;
		break;
	case 3:
		duty.u8low = PWM3L;
		duty.u8high = PWM3H;
		break;
	case 4:
		sfr_page(1);
		duty.u8low = PWM4L;
		duty.u8high = PWM4H;
		sfr_page(0);
		break;
	case 5:
		sfr_page(1);
		duty.u8low = PWM5L;
		duty.u8high = PWM5H;
		sfr_page(0);
		break;
	default:
		return 0;
	}
	return duty.u16;
}

/** controls PNP (PWM Negative Polarity) register */
void pwm_channel_set_polarity(uint8_t channel, uint8_t negative)
{
	channel = 1 << channel;
	if (negative)
		PNP |= channel;
	else
		PNP &= ~channel;
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
	pwm_channel_set_level(channel, mode);
	return;
}

enum pwm_mode_t pwm_channel_get_mode(uint8_t channel)
{
	if (PMEN & (1 << channel)) {
		if (PMD & (1 << channel))
			return PWM_MODE_HIGH;
		return PWM_MODE_LOW;
	}
	return PWM_MODE_RUN;
}
