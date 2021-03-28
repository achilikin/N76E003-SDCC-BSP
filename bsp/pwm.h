/*
  The MIT License (MIT)

  Simple routines to manipulate N76E003 PWM configuration.
  Not all available PWM features are enabled.
*/
#ifndef N76E003_PWM_H
#define N76E003_PWM_H

#include <N76E003.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_NUM_CHANNELS 6

#define PWM_POLARITY_POSITIVE 0
#define PWM_POLARITY_NEGATIVE 1

uint8_t pwm_channel_get_polarity(uint8_t channel);
void pwm_channel_set_polarity(uint8_t channel, uint8_t negative);

void pwm_channel_enable(uint8_t channel, bool enable);
uint8_t pwm_channel_is_enabled(uint8_t channel);
void pwm_channel_set_level(uint8_t channel, uint8_t set);

enum pwm_mode_t {
	PWM_MODE_LOW = 0,
	PWM_MODE_HIGH = 1,
	PWM_MODE_RUN = 2,
};

enum pwm_mode_t pwm_channel_get_mode(uint8_t channel);
void pwm_channel_set_mode(uint8_t channel, enum pwm_mode_t mode);

#define pwm_clock_tim1() (CKCON |= CKCON_PWMCKS) /** set PWM clock source to timer #1 */
#define pwm_clock_fsys() (CKCON &= ~CKCON_PWMCKS) /** set PWM clock source to Fsys */
#define pwm_start() PWMRUN = 1
#define pwm_stop()  PWMRUN = 0
#define pwm_clear() CLRPWM = 1

/** reads from PWM#L/PWM#H registers */
uint16_t pwm_duty_get(uint8_t channel);

/** writes to PWM#L/PWM#H registers, does not set LOAD */
void pwm_duty_set(uint8_t channel, uint16_t duty);

#ifdef __cplusplus
}
#endif
#endif
