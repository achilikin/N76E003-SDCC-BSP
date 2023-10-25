#include <N76E003.h>

#include <adc.h>
#include <irq.h>
#include <pwm.h>
#include <tick.h>
#include <uart.h>
#include <terminal.h>


#include "main.h"

#define APP_VERSION "2310.25"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"vdd [$val]\n"
	"pwr [auto|$pwr]\n"
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
		if (*arg)
			goto EARG;
		uart_putsc("\nresetting...\n");
		while (!uart_tx_empty());
		sw_reset();
	}

	if (str_is(cmd, "pwr")) {
		if (*arg == '\0') {
			if (power > 100) {
				uart_putsc("Auto: ");
				uart_putn(pwm_duty_get(LED_PWM_CHANNEL));
			} else {
				uart_putsc("Manual: ");
				uart_putn(power);
			}
			uart_putsc(" %\n");
			goto EOK;
		}

		if (str_is(arg, "auto")) {
			power = LED_AUTO_POWER;
			goto EOK;
		}

		val.u16 = argtou(arg, &arg);
		if (val.u16 > 100)
			power = LED_AUTO_POWER;
		else
			power = val.u8low;
		goto EOK;
	}

	if (str_is(cmd, "vdd")) {
		if (*arg) { /* calculate PWM duty for the given Vdd */
			val.u16 = argtou(arg, &arg);
			if (val.u16 < 300)
				val.u16 = 300;
			if (val.u16 > 420)
				val.u16 = 420;

			uart_putsc("Vdd: ");
			uart_putn(val.u16 / 100);
			uart_putc('.');
			uart_putn(val.u16 % 100);
			uart_putsc(" V, PWM: ");
			val.u8low = get_pwm_power(val.u16);
			uart_putn(val.u8low);
			uart_putsc(" %\n");
			goto EOK;
		}

		val.u16 = adc_get_vdd(ADC_GET_VDD);
		uart_putsc("Vdd: ");
		uart_putn(val.u16);
		uart_putsc(" mV, PWM: ");
		val.u8low = get_pwm_power(val.u16 / 10);

		uart_putn(val.u8low);
		uart_putsc(" %\n");

		pwm_duty_set(LED_PWM_CHANNEL, val.u8low);
		pwm_load();
		goto EOK;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
EOK:
	return CLI_EOK;
}
