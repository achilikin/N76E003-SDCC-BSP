#ifndef N76E003_ADC_H
#define N76E003_ADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_GET_RAW_BGAP 0
#define ADC_GET_BGAP	 1
#define ADC_GET_VDD		 2

uint16_t get_vdd(uint8_t mode);

#ifdef __cplusplus
}
#endif
#endif