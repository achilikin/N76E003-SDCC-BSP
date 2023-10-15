#include <N76E003.h>

#include "adc.h"
#include "iap.h"

static uint16_t iap_bgap = 0;

uint16_t adc_get_vdd(uint8_t mode)
{
	if (iap_bgap == 0) {
		iap_bgap = iap_read_uid(IAP_UID_SIZE);
		iap_bgap <<= 4;
		iap_bgap |= iap_read_uid(IAP_UID_SIZE + 1) & 0x0F;
	}

	if (mode == ADC_GET_RAW_BGAP)
		return iap_bgap;

	float fgap = (float)iap_bgap * 3072.0;
	fgap /= 4096.0;

	uint16_t bgap = (uint16_t)fgap;
	if (mode == ADC_GET_BGAP)
		return bgap;

	/* mode == ADC_GET_VDD */
	/* read ADC synchronously, takes up to 400us */
	Enable_ADC_BandGap;
	for (uint8_t i = 0; i < 5; i++) {
		clr_ADCF;
		set_ADCS;
		while (ADCF == 0);
	}
	/* convert to mV */
	bgap = (ADCRH << 4) + ADCRL;
	fgap = (4095.0 / bgap) * fgap;
	bgap = (uint16_t)fgap;
	return bgap;
}
