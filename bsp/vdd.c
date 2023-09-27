#include <N76E003.h>

#include "adc.h"
#include "iap.h"

uint16_t adc_get_vdd(uint8_t mode)
{
	uint16_t bgap = iap_read_uid(IAP_UID_SIZE);
	bgap <<= 4;
	bgap |= iap_read_uid(IAP_UID_SIZE + 1) & 0x0F;
	if (mode == ADC_GET_RAW_BGAP)
		return bgap;

	float fgap = (float)bgap * 3072.0;
	fgap /= 4096.0;
	bgap = (uint16_t)fgap;
	if (mode == ADC_GET_BGAP)
		return bgap;

	/* mode == ADC_GET_VDD */
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
