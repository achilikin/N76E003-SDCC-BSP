#include <N76E003.h>
#include <irq.h>
#include <pwm.h>
#include <tick.h>
#include <uart.h>
#include <terminal.h>


#include "main.h"

#define APP_VERSION "2103.28"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"reset\n"												   /* sw reset */
	"div [1|2|4|8|16|32|64|128]\n"							   /* set/get Fsys divider */
	"period [$u16]\n"										   /* get/set PWM period value */
	"type [edge|center]\n"									   /* set get PWM generation type */
	"pwm [start|stop]\n"									   /* start/stop PWM generation */
	"duty $0-5 [$u16]\n"									   /* get/set duty value for a channel */
	"mask $0-5 [pwm|low|high]\n"							   /* get/set mask for a channel */
	"gpmode [enable|disable]\n"								   /* group mode */
	"inttype [rise|fall|center|end|period]\n"				   /* PWM interrupt type, 'period' - sw */
	"opmode [independent|complementary|synchronized|phased]\n" /* operation mode, 'phased' - sw */
	"phases [$start $end]\n"								   /*get/set channels for phases range */
	"shift [0-255]\n"										   /* phased opmode shift between phased */
	;

val16_t val;

extern uint8_t pmd_end;
extern uint8_t pmd_start;
extern uint8_t phase_shift;

void print_type(void);
void print_opmode(void);
void print_gpmode(void);
void print_period(void);
void print_inttype(void);

int8_t commander(__idata char *cmd) {
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

	if (str_is(cmd, "pwm")) {
		if (*arg == '\0') {
			uart_putsc(PWMRUN ? "running" : "stopped");
			uart_putc(' ');
			uart_putsc("div");
			uart_putc(':');
			uart_putn(1 << (PWMCON1 & 0x07));
			uart_putc(' ');
			uart_putsc("period");
			uart_putc(':');
			print_period();
			uart_putc(' ');
			uart_putsc("type");
			uart_putc(':');
			print_type();
			uart_putc(' ');
			uart_putsc("opmode");
			uart_putc(':');
			print_opmode();
			uart_putc(' ');
			uart_putsc("inttype");
			uart_putc(':');
			print_inttype();
			uart_putsc(" group:");
			print_gpmode();
			uart_putc('\n');
			for (i = 0; i < 6; i++) {
				val.u16 = pwm_duty_get(i);
				uart_putsc("channel ");
				uart_putn(i);
				uart_putc(' ');
				uart_putrn(val.u16);
				uart_putc(' ');
				val.u8low = pwm_channel_get_mode(i);
				if (val.u8low == PWM_MODE_RUN)
					uart_putsc("pwm");
				else if (val.u8low == PWM_MODE_LOW)
					uart_putsc("low");
				else
					uart_putsc("high");
				uart_putc('\n');
			}
			goto EOK;
		}

		if (str_is(arg, "stop")) {
			PWMRUN = 0;
			if (is_opmode_phased())
				PMD = 0x00;	 /* set all outputs to 0, interrupt will driver them up */
		}
		else if (str_is(arg, "start"))
			PWMRUN = 1;
		else
			goto EARG;
		goto EOK;
	}

	if (str_is(cmd, "div")) {
		if (*arg == '\0') {
			uart_putn(1<< (PWMCON1 & 0x07));
			uart_putc('\n');
			goto EOK;
		}
		switch (argtou(arg, &arg)) {
		case 1:
			PWM_CLOCK_DIV_1;
			break;
		case 2:
			PWM_CLOCK_DIV_2;
			break;
		case 4:
			PWM_CLOCK_DIV_4;
			break;
		case 8:
			PWM_CLOCK_DIV_8;
			break;
		case 16:
			PWM_CLOCK_DIV_16;
			break;
		case 32:
			PWM_CLOCK_DIV_32;
			break;
		case 64:
			PWM_CLOCK_DIV_64;
			break;
		case 128:
			PWM_CLOCK_DIV_128;
			break;
		default:
			return CLI_EARG;
		}
		goto EOK;
	}

	if (str_is(cmd, "opmode")) { /* 17.1.3 Operation Modes */
		if (*arg == '\0') {
			print_opmode();
			uart_putc('\n');
			goto EOK;
		}
		if (str_is(arg, "independent"))
			PWMCON1 &= 0x3F;
		else if (str_is(arg, "complementary")) {
			i = PWMCON1;
			i &= 0x3F;
			i |= 0x40;
			PWMCON1 = i;
		} else if (str_is(arg, "synchronized"))
			PWMCON1 = (PWMCON1 & 0x3F) | 0x80;
		else if (str_is(arg, "phased")) {
			set_opmode_phased();
			PMEN = 0x3F; /* mask off all outputs */
			PMD = 0x00;	 /* set all outputs to 0, interrupt will driver them up */
			goto EOK;
		} else
			return CLI_EARG;
		clr_opmode_phased();
		PMEN = 0x00; /* run PWM for all outputs */
		goto EOK;
	}

	/* Group Mode. If enabled, the duty of the first three PWM pairs defined by PWM01H & PWM01L */
	if (str_is(cmd, "gpmode")) {
		if (*arg == '\0') {
			print_gpmode();
			uart_putc('\n');
			goto EOK;
		}
		if (str_is(arg, "enable"))
			PWMCON1 |= PWMCON1_GP;
		else if (str_is(arg, "disable"))
			PWMCON1 &= ~PWMCON1_GP;
		else
			return CLI_EARG;
		goto EOK;
	}

	if (str_is(cmd, "period")) {
		if (*arg == '\0') {
			print_period();
			uart_putc('\n');
			goto EOK;
		}
		val.u16 = argtou(arg, &arg);
		if (!val)
			return CLI_EARG;
		PWMPH = val.u8high;
		PWMPL = val.u8low;
		LOAD = 1;
		/* LOAD will be reset by HW only if PWM generator is running */
		if (PWMRUN)	while(LOAD);
		goto EOK;
	}

	if (str_is(cmd, "inttype")) {
		if (*arg == '\0') {
			print_inttype();
			uart_putc('\n');
			goto EOK;
		}
		if (str_is(arg, "fall"))
			i = 0x00;
		else if (str_is(arg, "rise"))
			i = SET_BIT4;
		else if (str_is(arg, "center"))
			i = SET_BIT5;
		else if (str_is(arg, "end"))
			i = SET_BIT5 | SET_BIT4;
		else if (str_is(arg, "period")) {
			set_inttype_period();
			goto EOK;
		} else
			return CLI_EARG;
		clr_inttype_period();
		sfr_page(1);
		PWMINTC &= ~(SET_BIT5 | SET_BIT4);
		PWMINTC |= i;
		sfr_page(0);
		goto EOK;
	}

	if (str_is(cmd, "duty")) {
		i = argtou(arg, &arg);
		if (i > 5)
			return CLI_EARG;
		if (*arg == '\0') {
			val.u16 = pwm_duty_get(i);
			uart_putnl(val.u16);
			goto EOK;
		}
		val.u16 = argtou(arg, &arg);
		if (!val)
			return CLI_EARG;
		pwm_duty_set(i, val.u16);
		LOAD = 1;
		if (PWMRUN)	while(LOAD);
		goto EOK;
	}

	if (str_is(cmd, "mask")) {
		i = argtou(arg, &arg);
		if (i > 5)
			return CLI_EARG;
		if (*arg == '\0') {
			val.u16 = pwm_duty_get(i);
			uart_putnl(val.u16);
			goto EOK;
		}
		if (str_is(arg, "pwm"))
			pwm_channel_set_mode(i, PWM_MODE_RUN);
		else if (str_is(arg, "low"))
			pwm_channel_set_mode(i, PWM_MODE_LOW);
		else if (str_is(arg, "high"))
			pwm_channel_set_mode(i, PWM_MODE_HIGH);
		else
			return CLI_EARG;
		goto EOK;
	}

	if (str_is(cmd, "type")) { /* 17.1.2 PWM Types */
		if (*arg == '\0') {
			print_type();
			uart_putc('\n');
			goto EOK;
		}
		if (str_is(arg, "edge"))
			PWM_EDGE_TYPE;
		else if (str_is(arg, "center"))
			PWM_CENTER_TYPE;
		else
			goto EARG;
		goto EOK;
	}

	/* phased mode settings */
	if (str_is(cmd, "phases")) {
		if (*arg == '\0') {
			for (i = 0; (1 << i) != pmd_start; i++);
			uart_putn(i);
			uart_putc(' ');
			for (; (1 << i) != pmd_end; i++);
			uart_putn(i);
			uart_putc('\n');
			goto EOK;
		}
		val.u8low = argtou(arg, &arg);
		if ((val.u8low > 5) || (*arg == '\0'))
			goto EARG;
		val.u8high = argtou(arg, &arg);
		if ((val.u8high > 5) || (val.u8high <= val.u8low))
			goto EARG;
		pmd_start = 1 << val.u8low;
		pmd_end = 1 << val.u8high;
		goto EOK;
	}

	/* shift between phases in number of PWM periods */
	if (str_is(cmd, "shift")) {
		if (*arg == '\0') {
			uart_putnl(phase_shift);
			goto EOK;
		}
		val.u16 = argtou(arg, &arg);
		if (val.u8high)
			goto EARG;
		phase_shift = val.u8low;
		goto EOK;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
EOK:
	return CLI_EOK;
}

void timer(void)
{
	return;
}

void print_opmode(void)
{
	uint8_t i = PWMCON1 >> 6;
	if (is_opmode_phased())
		uart_putsc("phased");
	else if (i == 0)
		uart_putsc("independent");
	else if (i == 1)
		uart_putsc("complementary"); /* PG0/2/4 are independent, PG1/3/5 out-phase of PG0/2/4, "dead-time" valid */
	else if (i == 2)
		uart_putsc("synchronized"); /* PG0/2/4 are independent, PG1/3/5 in-phase of PG0/2/4 */
	else
		uart_putsc("reserved");
	return;
}

void print_gpmode(void)
{
	if (PWMCON1 & PWMCON1_GP)
		uart_putsc("enabled");
	else
		uart_putsc("disabled");
	return;
}

void print_period(void)
{
	val.u8high = PWMPH;
	val.u8low = PWMPL;
	uart_putn(val.u16);
	return;
}

void print_inttype(void)
{
	if (is_inttype_period()) {
		uart_putsc("period");
		return;
	}
	sfr_page(1);
	uint8_t i = PWMINTC;
	sfr_page(0);
	i = (i >> 4) & 0x03;
	if (i == 0)
		uart_putsc("fall");
	else if (i == 1)
		uart_putsc("rise");
	else if (i == 2)
		uart_putsc("center");
	else
		uart_putsc("end");
	return;
}

void print_type(void)
{
	if (PWMCON1 & SET_BIT4)
		uart_putsc("center");
	else
		uart_putsc("edge");
	return;
}