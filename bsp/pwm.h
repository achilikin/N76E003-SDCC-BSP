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

/**
 * Supported PWM ranges
 */
#define PWM_RANGE_1HZ		1 /** .001 to .099 kHz */
#define PWM_RANGE_100HZ		2 /** .100 to .999 kHz */
#define PWM_RANGE_1KHZ		3 /** 1.00 to 9.99 kHz */
#define PWM_RANGE_10KHZ		4 /** 10.0 to 99.9 kHz */
#define PWM_RANGE_100KHZ	5 /** 100. to 160. kHz */
#define PWM_RANGE_NUM		6

#define PWM_POLARITY_POSITIVE 0
#define PWM_POLARITY_NEGATIVE 1

/** will initialize TIM1 as PWM clock source */
void pwm_init_timer(void);

/**
 * @param pwm_freq : 3 digit value for selected range, for example:
 * 		  777 for PWM_RANGE_100HZ means 777. Hz
 * 		  777 for PWM_RANGE_10KHZ means 7.77 kHz
 * @param range one of PWM_RANGE_* values
 */
void pwm_set_freq(uint16_t pwm_freq, uint8_t range);

void pwm_channel_init(uint8_t channel, uint8_t duty);
void pwm_channel_set_duty(uint8_t channel, uint8_t duty);
uint8_t pwm_channel_get_polarity(uint8_t channel);
void pwm_channel_set_polarity(uint8_t channel, uint8_t negative);

void pwm_channel_enable(uint8_t channel, bool enable);
uint8_t pwm_channel_is_enabled(uint8_t channel);
void pwm_channel_set_level(uint8_t channel, uint8_t set);

enum pwm_mode_t {
	PWM_MODE_RUN = 1,
	PWM_MODE_LOW = 2,
	PWM_MODE_HIGH = 3
};
void pwm_channel_set_mode(uint8_t channel, enum pwm_mode_t mode);

#define pwm_start() PWMRUN = 1
#define pwm_stop()  PWMRUN = 0
#define pwm_clear() CLRPWM = 1

uint16_t pwm_str2freq(__idata char *str);

#ifdef __cplusplus
}
#endif
#endif
