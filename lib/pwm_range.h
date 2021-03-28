/*
  The MIT License (MIT)

  Simple routines providing PWM ranges for N76E003 PWM configuration.
*/
#ifndef N76E003_PWM_RANGE_H
#define N76E003_PWM_RANGE_H

#include <pwm.h>

#ifdef __cplusplus
extern "C" {
#endif

/** will initialize TIM1 as PWM clock source */
void pwm_init_timer(void);

/**
 * Supported PWM ranges
 */
#define PWM_RANGE_1HZ		1 /** .001 to .099 kHz */
#define PWM_RANGE_100HZ		2 /** .100 to .999 kHz */
#define PWM_RANGE_1KHZ		3 /** 1.00 to 9.99 kHz */
#define PWM_RANGE_10KHZ		4 /** 10.0 to 99.9 kHz */
#define PWM_RANGE_100KHZ	5 /** 100. to 160. kHz */
#define PWM_RANGE_NUM		6

/**
 * @param pwm_freq : 3 digit value for selected range, for example:
 * 		  777 for PWM_RANGE_100HZ means 777. Hz
 * 		  777 for PWM_RANGE_10KHZ means 7.77 kHz
 * @param range one of PWM_RANGE_* values
 */
void pwm_set_freq(uint16_t pwm_freq, uint8_t range);

void pwm_channel_init(uint8_t channel, uint8_t duty);

/** sets duty in percents */
void pwm_channel_set_duty(uint8_t channel, uint8_t duty);

#ifdef __cplusplus
}
#endif
#endif
