#ifndef MAIN_H
#define MAIN_H

#include <N76E003.h>

#ifdef MARK_PIN
/** set pin to high and back to trace a time marker on oscilloscope */
#define MARK do{MARK_PIN=1;MARK_PIN=0;}while(0)
#define MARK_ON MARK_PIN=1
#define MARK_OFF MARK_PIN=0
#else
#define MARK
#define MARK_ON
#define MARK_OFF
#endif

int8_t commander(__idata char *cmd); /** cli handler */
void timer(void); /** timer handler called every second if enabled */

void pwm_interrupt_handler(void) INTERRUPT(IRQ_PWM, IRQ_PWM_REG_BANK);

/* GF0 used to indicate SW defined 'period' interrupt type*/
#define set_inttype_period() set_gf0()
#define clr_inttype_period() clr_gf0()
#define is_inttype_period() get_gf0()

/* GF1 used to indicate SW defined 'phased' operation mode */
#define set_opmode_phased() set_gf1()
#define clr_opmode_phased() clr_gf1()
#define is_opmode_phased() get_gf1()

#endif
