#ifndef MAIN_H
#define MAIN_H

#include <N76E003.h>

#define EPOLL_PIN P15 /** event processing loop poll pin output */
#define MARK_PIN  P10 /** pin to set time markers */

/** set pin to high and back to trace a time marker on oscilloscope */
#define MARK do{MARK_PIN=1;MARK_PIN=0;}while(0)

int8_t test_cli(__idata char *cmd); /** cli handler */
void timer(void); /** timer handler called every second if enabled */
void set_rc_trim(uint8_t rctrim); /** update RC trim value */

#endif
