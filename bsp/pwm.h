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
#define pwm_clear() do{CLRPWM=1;while(CLRPWM);}while(0)
#define pwm_load() do{LOAD=1;if (PWMRUN)while(LOAD);}while(0)

/**
 * writes to PWMPL/PWMPH registers, does not set LOAD.
 * For edge aligned PWM frequency == Fpwm /({PWMPH,PWNPL} + 1),
 * for center aligned PWM frequency == Fpwm /(2 * {PWMPH,PWNPL})
 */
#define pwm_period_set(x) do{PWMPL=LOBYTE(x);PWMPH=HIBYTE(x);}while(0)

/** reads from PWMPL/PWMPH registers */
uint16_t pwm_period_get(void);

/** reads from PWM#L/PWM#H registers */
uint16_t pwm_duty_get(uint8_t channel);

/** writes to PWM#L/PWM#H registers, does not set LOAD */
void pwm_duty_set(uint8_t channel, uint16_t duty);

enum pwm_irq_type_t {
	PWM_IRQ_FALL = 0,
	PWM_IRQ_RISE = SET_BIT4,
	PWM_IRQ_CENTER = SET_BIT5,
	PWM_IRQ_END = SET_BIT5 | SET_BIT4,
};

enum pwm_irq_select_t {
	PWM_IRQ_PWM0 = 0,
	PWM_IRQ_PWM1 = 1,
	PWM_IRQ_PWM2 = 2,
	PWM_IRQ_PWM3 = 3,
	PWM_IRQ_PWM4 = 4,
	PWM_IRQ_PWM5 = 5,
};

void pwm_irq_set_type(enum pwm_irq_type_t type);

#ifdef __cplusplus
}
#endif
#endif
