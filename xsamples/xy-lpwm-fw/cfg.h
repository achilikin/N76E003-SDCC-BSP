/*
  The MIT License (MIT)

  An example how application can store configuration in the
  flash memory of N76E003 MC
*/
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define CGF_PWM_RUN    0x01 /**< PWM running  */
#define CGF_PWM_DIRECT 0x02 /**< PWM in direct mode, not negative */

typedef struct {
	uint8_t  duty;	/* stored PWM duty 0 to 100 */
	uint16_t freq;	/* stored PWM frequency */
	uint8_t  range;	/* stored PWM range */
	uint8_t  flags;
} cfg_t;

extern cfg_t cfg;

/** loads  configuration from the flash memory */
void cfg_load(void);

/** saves any changes to the flash memory */
void cfg_save(void);

#endif

