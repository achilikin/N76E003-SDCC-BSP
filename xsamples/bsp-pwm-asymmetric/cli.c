#include <N76E003.h>
#include <irq.h>
#include <pwm.h>
#include <tick.h>
#include <uart.h>
#include <terminal.h>


#include "main.h"

#define APP_VERSION "2307.05"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"reset\n"			/* sw reset */
	"adc [on|off]\n"
	"duty $0-1 [$u8]\n" /* get/set duty value for a channel */
	"mode [independent|inphase|antiphase]\n"
	;

val16_t val;

void print_mode(void);

int8_t commander(__idata char* cmd)
{
	uint8_t i;
	__idata char *arg = get_arg(cmd);

	if (str_is(cmd, "help")) {
		uart_putsc("VER: ");
		uart_putsc(APP_VERSION);
		uart_putsc(" (");
		uart_putn(sdcc_get_code_size());
		uart_putsc(" bytes)\n");

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

	if (str_is(cmd, "duty")) {
		i = argtou(arg, &arg);
		if (i > PWM_MAX_CHANNEL)
			return CLI_EARG;

		if (*arg == '\0') {
			val.u16 = pwm_duty_get(i);
			if (i == 1) /* normalize CH1 value to PWM_PERIOD/2 */
				val.u8low -= PWM_DUTY_MAX;
			uart_putnl(val.u16);
			goto EOK;
		}

		val.u16 = argtou(arg, &arg);
		if (val.u8high || val.u8low > PWM_DUTY_MAX)
			goto EARG;

		pwm_set_signal_duty(i, val.u8low);
		goto EOK;
	}

	if (str_is(cmd, "adc")) {
		if (*arg == '\0') {
			if (adc_mode_get())
				uart_putsc("on");
			else
				uart_putsc("off");
			uart_putsc(" (");
			uart_putn(adc_duty);
			uart_putsc(")\n");
			goto EOK;
		}
		if (str_is(arg, "on"))
			adc_mode_set();
		else if (str_is(arg, "off"))
			adc_mode_clr();
		else
			goto EARG;
		goto EOK;
	}

	if (str_is(cmd, "mode")) {
		if (*arg == '\0') {
			print_mode();
			uart_putc('\n');
			goto EOK;
		}
		if (str_is(arg, "independent"))
			pwm_signal_mode = PWM_SIGNAL_INDEPENDENT;
		else if (str_is(arg, "inphase"))
			pwm_signal_mode = PWM_SIGNAL_INPHASE;
		else if (str_is(arg, "antiphase"))
			pwm_signal_mode = PWM_SIGNAL_ANTIPHASE;
		else
			goto EARG;
		goto EOK;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
EOK:
	return CLI_EOK;
}

void print_mode(void)
{
	if (pwm_signal_mode == PWM_SIGNAL_INDEPENDENT)
		uart_putsc("independent");
	else if (pwm_signal_mode == PWM_SIGNAL_INPHASE)
		uart_putsc("inphase");
	else
		uart_putsc("antiphase");
	return;
}