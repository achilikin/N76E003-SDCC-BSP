#ifndef N76E003_ADC_H
#define N76E003_ADC_H

#include <N76E003.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_GET_RAW_BGAP 0
#define ADC_GET_BGAP	 1
#define ADC_GET_VDD		 2

enum adc_channel_t {
	ADC_AIN0, /* P0.7, pin 6 */
	ADC_AIN1, /* P3.0, pin 5 */
	ADC_AIN2, /* P0.7, pin 3 (UART RXD) */
	ADC_AIN3, /* P0.6, pin 2 (UART TXD) */
	ADC_AIN4, /* P0.5, pin 1 */
	ADC_AIN5, /* P0.4, pin 20 */
	ADC_AIN6, /* P0.3, pin 19  */
	ADC_AIN7, /* P1.1, pin 14 */
	ADC_BAND_GAP /* internal */
};

#define adc_enable() ADCCON1|=SET_BIT0
#define adc_disable() ADCCON1&=~SET_BIT0
#define adc_start() ADCS=1
#define adc_busy() ADCS
#define adc_done() ADCF
#define adc_clear() ADCF=0
#define adc_select_channel(x) do{ADCCON0&=0xF0;ADCCON0|=x;}while(0)

/* adc.c */
uint16_t adc_read(void);

/* vdd.c, requires iap_read.c */
uint16_t get_vdd(uint8_t mode);

#ifdef __cplusplus
}
#endif
#endif