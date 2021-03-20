#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include <lcd_lpwm.h>
#include "target.h"

/**
 * cli parser
 */
int8_t lpwm_cli(__idata char *cmd);

void set_pwm_duty(uint8_t duty);
void print_duty(uint8_t duty);
void print_range(uint8_t duty);
void print_off(uint8_t dstart);
#define print_freq(freq) lcd_printn(freq, 1, 3)

#endif

