#ifndef MAIN_H
#define MAIN_H

#include <N76E003.h>

#ifdef MARK_PIN
/** set pin to high and back to trace a time marker on oscilloscope */
#define MARK do{MARK_PIN=1;MARK_PIN=0;}while(0)
#else
#define MARK
#endif


int8_t commander(__idata char *cmd); /** cli handler */

void timer(void); /** timer handler called every second if enabled */
void timer_reset(void); /** restart timer counter */

#endif
