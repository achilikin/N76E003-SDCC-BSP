#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#include "target.h"

/** cli parser */
int8_t lcd_cli(__idata char *cmd);

void set_pwm_duty(uint8_t duty);

#endif

