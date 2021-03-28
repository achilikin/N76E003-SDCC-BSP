/*
  The MIT License (MIT)

  An example how application can store configuration in the
  flash memory of N76E003 MC
*/
#include <stdint.h>

#include <N76E003.h>
#include <iap.h>

#include "cfg.h"
#include "pwm_range.h"

cfg_t cfg;

/* last flash memory page will be used for storing configuration */
#define CFG_ADDRESS APROM_SIZE - PAGE_SIZE

/* address of the configuration structure in the flash memory */
AT_ADDRESS(CFG_ADDRESS) cfg_t ecfg;

void cfg_save(void)
{
	/* save only if changes are detected */
	if ((cfg.duty == ecfg.duty) && (cfg.freq == ecfg.freq)
	 	&& (cfg.range == ecfg.range)  && (cfg.flags == ecfg.flags))
		return;

	/* erase the page if needed */
	if (ecfg.duty != 0xFF)
		iap_erase_aprom(CFG_ADDRESS);

	/* copy config to the flash */
	uint8_t	*data = (uint8_t *)&cfg;
	for (uint8_t i = 0; i < sizeof(cfg); i++)
		iap_prog_aprom(CFG_ADDRESS + i, data[i]);
}

void cfg_load(void)
{
	/*
	 * if duty == 0xFF then config was never been saved before
	 * and needs to be initialized with the default values
	 * */
	if (ecfg.duty == 0xFF) {
		cfg.duty = 50;
		cfg.freq = 100;
		cfg.range = PWM_RANGE_1KHZ;
		cfg.flags = CGF_PWM_RUN;
		cfg_save();
	} else {
		cfg.duty = ecfg.duty;
		cfg.freq = ecfg.freq;
		cfg.range = ecfg.range;
		cfg.flags = ecfg.flags;
	}
}
