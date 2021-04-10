#include <N76E003.h>

#include <iap.h>
#include <irq.h>
#include <adc.h>
#include <dump.h>
#include <uart.h>
#include <lcd_lpwm.h>
#include <terminal.h>

#include "main.h"
#include "cfg.h"
#include "pwm_range.h"

#define APP_VERSION "2104.10"

#if (TARGET_BOARD == VHM_800_BOARD)
	#define BOARD_NAME "VHM-800"
#elif (TARGET_BOARD == NONAME_BOARD)
	#define BOARD_NAME "NONAME"
#else
	#define BOARD_NAME "XY-LPWM"
#endif

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"reset\n"
	"pwm\n"
	"duty 0-100\n"
	"freq k001-160k\n"
	"out on|off|negative|direct\n";

/* cli parser */
int8_t lpwm_cli(__idata char *cmd)
{
	uint8_t i;
	uint16_t freq;
	__idata char *arg = get_arg(cmd);

	if (str_is(cmd, "help")) {
		uart_putsc("VER: ");
		uart_putsc(BOARD_NAME);
		uart_putc(' ');
		uart_putsc(APP_VERSION);
		uart_putsc(" (");
		uart_putn(sdcc_get_code_size());
		uart_putsc(" bytes)\n");

		uart_putsc("BGP: ");
		uint16_t bgap = get_vdd(ADC_GET_RAW_BGAP);
		uart_puthw(bgap);
		uart_putc(' ');
		bgap = get_vdd(ADC_GET_BGAP);
		uart_putn(bgap);
		uart_putsc("mV\nVDD: ");
		bgap = get_vdd(ADC_GET_VDD);
		uart_putn(bgap);
		uart_putsc("mV\nUID:");

		for (i = 1; i <= IAP_UID_SIZE; i++) {
			uart_putc(' ');
			uart_puth(iap_read_uid(IAP_UID_SIZE-i));
		}
		uart_putc('\n');
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

	/* SW reset */
	if (str_is(cmd, "reset")) {
		uart_putsc("resetting...");
		while (!uart_tx_empty());
		sw_reset();
	}

	if (str_is(cmd, "duty")) {
		uint8_t duty = argtou(arg, &arg);
		if (duty > 100)
			goto EARG;
		set_pwm_duty(duty);
		print_duty(duty);
		cfg.duty = duty;
		cfg_save();
		goto EOK;
	}

	if (str_is(cmd, "freq")) {
		freq = pwm_str2freq(arg);
		if (freq == 0)
			goto EARG;
		pwm_set_freq(freq & 0x3FF, freq >> 12);
		cfg.freq = freq & 0x3FF;
		cfg.range = freq >> 12;
		print_freq(cfg.freq);
		print_range(cfg.range);
		goto SAVE;
	}

	if (str_is(cmd, "out")) {
		if (str_is(arg, "on")) {
			pwm_channel_enable(PWM_CHANNEL, true);
			pwm_start();
			print_freq(cfg.freq);
			print_range(cfg.range);
			print_duty(cfg.duty);
			lcd_set_sign(LCD_SIG_OUT, true);
			lcd_set_sign(LCD_SIG_PERCENT, true);
			cfg.flags |= CGF_PWM_RUN;
			goto SAVE;
		}
		if (str_is(arg, "off")) {
			pwm_stop();
			pwm_channel_enable(PWM_CHANNEL, false);
			pwm_channel_set_level(PWM_CHANNEL, !(cfg.flags & CGF_PWM_DIRECT));
			lcd_set_sign(LCD_SIG_OUT, false);
			cfg.flags &= ~CGF_PWM_RUN;
			goto SAVE;
		}
		if (str_is(arg, "negative")) {
			pwm_channel_set_polarity(PWM_CHANNEL, PWM_POLARITY_NEGATIVE);
			cfg.flags &= ~CGF_PWM_DIRECT;
			goto SAVE;
		}
		if (str_is(arg, "direct")) {
			pwm_channel_set_polarity(PWM_CHANNEL, PWM_POLARITY_POSITIVE);
			cfg.flags |= CGF_PWM_DIRECT;
			goto SAVE;
		}
	}

	if (str_is(cmd, "pwm")) {
		if (cfg.flags & CGF_PWM_RUN)
			uart_putsc("on");
		else
			uart_putsc("off");
		uart_putsc(" freq: ");
		if (cfg.range <= PWM_RANGE_100HZ)
			uart_putc('k');
		freq = cfg.freq;
		uart_putn(freq/100);
		if (cfg.range == PWM_RANGE_1KHZ)
			uart_putc('k');
		freq = freq % 100;
		uart_putn(freq/10);
		if (cfg.range == PWM_RANGE_10KHZ)
			uart_putc('k');
		freq = freq % 10;
		uart_putn(freq);
		if (cfg.range == PWM_RANGE_100KHZ)
			uart_putc('k');
		uart_putsc(", duty: ");
		uart_putn(cfg.duty);
		if (pwm_channel_get_polarity(PWM_CHANNEL))
			uart_putsc(" negative");
		else
			uart_putsc(" direct");
		uart_putc('\n');

		goto EOK;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
SAVE:
	cfg_save();
EOK:
	return CLI_EOK;
}
