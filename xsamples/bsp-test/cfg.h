/*
  The MIT License (MIT)

  An example how application's configuration can be stored in the
  flash memory of N76E003
*/
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

/**
 * some configuration flags, should not add up to 0xFF
 * 0xFF is used as initialization marker
 */
#define CFG_TIMER_ON	0x01 /** call timer handler every second */
#define CFG_OUT_LCD		0x02 /** timer handler outputs to LCD */
#define CFG_OUT_UART 	0x04 /** timer outputs to serial port */
#define CFG_NO_KBD		0x08 /** PS/2 keyboard not connected */

typedef struct {
	uint8_t flags;
	uint8_t trim;
} cfg_t;

extern cfg_t cfg;

void cfg_load(void);
void cfg_save(void);

#endif

