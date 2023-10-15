#include <N76E003.h>

#include <adc.h>
#include <irq.h>
#include <pwm.h>
#include <tick.h>
#include <uart.h>
#include <terminal.h>


#include "main.h"

#define APP_VERSION "2310.15"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"vdd\n"
	"reset\n" /* sw reset */
;

val16_t val;

int8_t commander(__idata char* cmd)
{
	__idata char *arg = get_arg(cmd);

	if (str_is(cmd, "help")) {
		uart_putsc("VER: ");
		uart_putsc(APP_VERSION);
		uart_putsc(" (");
		uart_putn(sdcc_get_code_size());
		uart_putsc(" bytes)\n");

		uart_putsc("BGP: "); /* ADC bandgap */
		uart_puthw(adc_get_vdd(ADC_GET_RAW_BGAP));
		uart_putc(' ');
		uart_putn(adc_get_vdd(ADC_GET_BGAP));
		uart_putsc(" mV\n");
		uart_putsc("Vdd: ");
		uart_putn(adc_get_vdd(ADC_GET_VDD));
		uart_putsc(" mV\n");

		uart_putsc("CMD:");
		__code char *list = cmd_list;
		for (;; list++) {
			uint8_t ch = *list;
			if (ch == 0)
				break;
			if (ch == '\n') {
				uart_putc('\n');
				uart_putsc("    ");
				continue;
			}
			uart_putc(ch);
		}
		uart_putc('\n');
		goto EOK;
	}

	if (str_is(cmd, "reset")) {
		uart_putsc("\nresetting...\n");
		while (!uart_tx_empty());
		sw_reset();
	}

	if (str_is(cmd, "vdd")) {
		if (*arg)
			goto EARG;

		val.u16 = adc_get_vdd(ADC_GET_VDD);
		uart_putsc("Vdd: ");
		uart_putn(val.u16);
		uart_putsc(" mV, PWM: ");
		/* normalize Vdd to 3.00 to 4.20V range */
		/* 3.00 = 100% PWM, 4.20 rounded to 70% PWM */
		val.u16 = val.u16 / 10;
		if (val.u16 < 300)
			val.u16 = 300;
		if (val.u16 > 420)
			val.u16 = 420;
		/* convert to 0 - 1.20 V range */
		val.u16 = val.u16 - 300;
		/* convert 0-1.20 V range to 0-30% range */
		val.u8low = val.u8low / 4;
		/* calculate PWM value */
		val.u8high = 100 - val.u8low;
		uart_putn(val.u8high);
		uart_putsc(" %\n");
		pwm_duty_set(LED_PWM_CHANNEL, val.u8high);
		pwm_load();
		goto EOK;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
EOK:
	return CLI_EOK;
}
