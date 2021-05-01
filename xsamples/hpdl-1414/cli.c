#include <N76E003.h>
#include <i2c.h>
#include <uart.h>
#include <tick.h>
#include <mcp23017.h>
#include <terminal.h>

#include "main.h"

#define APP_VERSION "2105.01"

/* list of supported commands */
const __code char cmd_list[] =
	"\n"
	"reset\n"						/* sw reset */
	"i2c scan\n"					/* scan I2C bus for valid addresses */
	"i2c speed 100|400|800|1000\n" 	/* I2C clock speed in kHz */
	"i2c wr $dev $val [$val ...]\n" /* write data */
	"i2c read $dev $addr [$len]\n"	/* read data with re-start */
	"hpdl @ACCCC\n"					/* A - address '0' to '3', C - character to display */
	"hpdl $addr $char\n"
	"hpdl test on|off\n";

uint8_t test = 0;

int8_t hpdl_write(uint8_t addr, uint8_t data);

int8_t commander(__idata char *cmd)
{
	uint8_t i, reg, n;
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

	if (str_is(cmd, "i2c")) {
		if (str_is(arg, "speed")) {
			arg = get_arg(arg);
			uint16_t clock = argtou(arg, &arg);
			if (clock == 100)
				i2c_init(I2C_CLOCK_100K, 5, false);
			else if (clock == 400)
				i2c_init(I2C_CLOCK_400K, 5, false);
			else if (clock == 800)
				i2c_init(I2C_CLOCK_800K, 5, false);
			else if (clock == 1000)
				i2c_init(I2C_CLOCK_1MHZ, 5, false);
			else
				goto EARG;
			goto EOK;
		}
		if (str_is(arg, "scan")) {
			for (i = 0x08; i < 0x7F; i++) {
				if (i2c_start(i << 1) == I2C_EOK) {
					uart_putsc("Found device: 0x");
					uart_puth(i);
					uart_putsc(" (0x");
					uart_puth(i << 1);
					uart_putsc(")");
					uart_putln();
				}
				i2c_stop();
			}
			goto EOK;
		}
		if (str_is(arg, "wr")) { /* write data */
			arg = get_arg(arg);
			i = argtou(arg, &arg);
			if (i2c_start(i << 1) == I2C_EOK) {
				while (*arg != '\0')
					i2c_write(argtou(arg, &arg));
			}
			i2c_stop();
			goto EOK;
		}
		if (str_is(arg, "read")) { /* read byte */
			arg = get_arg(arg);
			i = argtou(arg, &arg);
			if (*arg == '\0')
				goto EARG;
			reg = argtou(arg, &arg);
			n = argtou(arg, &arg);
			if (n == 0)
				n = 1;
			if (i2c_start(i << 1) == I2C_EOK) {
				i2c_write(reg);
				if (i2c_start((i << 1) | I2C_READ) == I2C_EOK) {
					while (n) {
						i = i2c_read(n > 1);
						uart_putsc(" x");
						uart_puth(i);
						n--;
					}
				}
			}
			i2c_stop();
			uart_putln();
			goto EOK;
		}
		goto EARG;
	}

	if (str_is(cmd, "hpdl")) {
		if (str_is(arg, "test")) {
			arg = get_arg(arg);
			if (str_is(arg, "on"))
				test = 1;
			else if (str_is(arg, "off"))
				test = 0;
			else
				goto EARG;
			if (test) {
				timer();
				timer_reset();
			} else {
				for (i = 0; i < 4; i++)
					hpdl_write(i, ' ');
			}
			goto EOK;
		}

		uint8_t addr;
		if (arg[0] == '@') {
			arg++;
			addr = arg[0];
			if ((addr < '0') || (addr > '3'))
				goto EARG;
			addr -= '0';
			arg++;
			for (i = 0; (addr < 4) && (i < 4); i++) {
				if (hpdl_write(addr ^ 0x03, arg[i]))
					return CLI_ENODEV;
				addr++;
			}
			goto EOK;
		}
		addr = argtou(arg, &arg);
		if (addr > 3)
			goto EARG;
		addr ^= 0x03; /* convert to left-to-right addressing */
		uint8_t val = arg[0];
		if (val < 0x21)
			val = ' ';
		/* only printable characters are allowed */
		if (val > 0x5F)
			goto EARG;
		if (hpdl_write(addr, val))
			return CLI_ENODEV;
		goto EOK;
	}

	return CLI_ENOTSUP;
EARG:
	return CLI_EARG;
EOK:
	return CLI_EOK;
}

int8_t hpdl_write(uint8_t addr, uint8_t data)
{
	if (i2c_start(I2C_MCP23017 | I2C_WRITE) == I2C_EOK) {
		i2c_write(MCPB0_GPIOA);
		i2c_write(data);		/* write data bus first */
		i2c_write(0xFC | addr); /* write address */
		i2c_stop();
		/* now toggle WR bit */
		i2c_start(I2C_MCP23017 | I2C_WRITE);
		i2c_write(MCPB0_GPIOB);
		i2c_write(0xF8 | addr); /* write address */
		i2c_stop();
		i2c_start(I2C_MCP23017 | I2C_WRITE);
		i2c_write(MCPB0_GPIOB);
		i2c_write(0xFC | addr); /* write address */
		i2c_stop();
		return 0;
	}
	i2c_stop();
	return -1;
}

const static uint8_t tval[3] = { 'O', '*', '.' };

void timer(void)
{
	if (test == 0)
		return;

	for (uint8_t i = 0; i < 4; i++)
		hpdl_write(i ^ 0x03, tval[test - 1]);

	test += 1;
	if (test > 3)
		test = 1;
}
