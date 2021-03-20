/*
  The MIT License (MIT)

  An example how application can store configuration in the
  flash memory of N76E003 MC
*/
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef struct {
	uint8_t duty;	/* backlight PWM duty 1 to 100 */
	uint8_t lcd_on; /* lcd state */
} cfg_t;

extern cfg_t cfg;

/** loads  configuration from the flash memory */
void cfg_load(void);

/** saves any changes to the flash memory */
void cfg_save(void);

#endif

