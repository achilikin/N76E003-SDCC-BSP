/*
  The MIT License (MIT)

  An example how application's configuration can be stored in the
  flash memory of N76E003
*/
#include <N76E003.h>

#include <iap.h>

#include "cfg.h"

cfg_t cfg;

/* last page will be used for storing configuration */
#define CFG_ADDRESS APROM_SIZE - PAGE_SIZE /* 0x4780 */

/* address of the configuration structure in the flash memory */
__code __at(CFG_ADDRESS) cfg_t ecfg;

void cfg_save(void)
{
	/* check for any changes */
	if ((cfg.flags == ecfg.flags) && (cfg.trim == ecfg.trim))
		return;

	/* erase the page if needed */
	if (ecfg.flags != 0xFF)
		iap_erase_aprom(CFG_ADDRESS);

	/* copy config to the flash */
	uint8_t	*data = (uint8_t *)&cfg;
	for (uint8_t i = 0; i < sizeof(cfg); i++)
		iap_prog_aprom(CFG_ADDRESS + i, data[i]);
}

void cfg_load(void)
{
	/* if flags == 0xFF then config not been saved yet */
	/* and needs to be initialized */
	if (ecfg.flags == 0xFF) {
		cfg.flags = CFG_OUT_LCD | CFG_OUT_UART;
		cfg.trim = HIRC_TRIM;
		cfg_save();
	} else {
		cfg.flags = ecfg.flags;
		cfg.trim = ecfg.trim;
	}
}
