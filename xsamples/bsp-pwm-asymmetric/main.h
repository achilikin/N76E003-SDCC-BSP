#ifndef MAIN_H
#define MAIN_H

#include <N76E003.h>

#ifdef MARK_PIN
/** set pin to high and back to trace a time marker on oscilloscope */
#define MARK do{MARK_PIN=1;MARK_PIN=0;}while(0)
#define MARK_ON MARK_PIN=1
#define MARK_OFF MARK_PIN=0
#define SYNC_ON  P03=1 /* start of the PWM period, used as sync signal for oscilloscope */
#define SYNC_OFF P03=0
#define SYNC_TOGGLE P03^=1
#define SYNC_PULSE do{P03=1;P03=0;}while(0)
#else
#define MARK
#define MARK_ON
#define MARK_OFF
#define SYNC_ON
#define SYNC_OFF
#define SYNC_TOGGLE
#define SYNC_PULSE
#endif

#define PWM_MAX_CHANNEL 2 /* 0 - phase 0, 1 - phase 1, 2 - centre point */

/**
 * ADC will be used to control PWM duty per channel.
 * 0 - 4095 ADC output will be normalized to 0 - 102.
 * So PWM period will be 102 * 2 = 204 and period and duty values will fit into 8 bits
 */
#define PWM_PERIOD 204
#define PWM_DUTY_MAX (PWM_PERIOD/2)

#define PIN_OUT0 P14
#define PIN_OUT1 P13

extern uint8_t adc_duty;

enum pwm_signal_mode_t {
	PWM_SIGNAL_INPHASE,
	PWM_SIGNAL_ANTIPHASE,
	PWM_SIGNAL_INDEPENDENT
};

extern uint8_t pwm_signal_mode;

void pwm_set_signal_duty(uint8_t channel, uint8_t duty);

/* GF0 used to store adc mode */
#define adc_mode_set() set_gf0()
#define adc_mode_get() get_gf0()
#define adc_mode_clr() clr_gf0()

int8_t commander(__idata char* cmd); /** cli handler */
void timer(void); /** timer handler called every second if enabled */

void pwm_interrupt_handler(void) INTERRUPT(IRQ_PWM, IRQ_PWM_REG_BANK);

#endif
