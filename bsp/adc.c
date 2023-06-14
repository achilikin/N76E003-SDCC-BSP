#include <N76E003.h>

#include "adc.h"

uint16_t adc_read(void)
{
	uint16_t adc_val = (ADCRH << 4) + ADCRL;
	return adc_val;
}

