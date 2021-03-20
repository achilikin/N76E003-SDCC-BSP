/*
  The MIT License (MIT)

  Command line processing

  Configuration defines (can be changed in Makefile):
    #define CMD_LEN 0x20 // must be power of two
*/
#include <N76E003.h>

#include "uart.h"
#include "terminal.h"

/* Escape sequence states */
#define ESC_CHAR    0
#define ESC_BRACKET 1
#define ESC_BRCHAR  2
#define ESC_TILDA   3
#define ESC_CRLF    5

uint8_t terminal_getc(uint8_t ch)
{
	static uint8_t esc = ESC_CHAR;
	static uint8_t idx = 0;

	/* ESC sequence state machine */
	if (ch == 27) {
		esc = ESC_BRACKET;
		return 0;
	}

	if (esc == ESC_BRACKET) {
		if (ch == '[') {
			esc = ESC_BRCHAR;
			return 0;
		}
	}

	if (esc == ESC_BRCHAR) {
		esc = ESC_CHAR;
		if (ch >= 'A' && ch <= 'D') {
			ch |= EXTRA_KEY;
			return ch;
		}
		if ((ch >= '1') && (ch <= '6')) {
			esc = ESC_TILDA;
			idx = ch - '0';
			return 0;
		}
		return ch;
	}
	if (esc == ESC_TILDA) {
		esc = ESC_CHAR;
		if (ch == '~') {
			ch = EXTRA_KEY | idx;
			return ch;
		}
		return 0;
	}

	/* convert CR to LF */
	if (ch == '\r') {
		esc = ESC_CRLF;
		return '\n';
	}
	/* do not return LF if it is part of CR+LF combination */
	if (ch == '\n') {
		if (esc == ESC_CRLF) {
			esc = ESC_CHAR;
			return 0;
		}
	}
	esc = ESC_CHAR;
	return ch;
}

__idata char *get_arg(__idata char *str)
{
	for (; *str && *str != ' '; str++);

	if (*str == ' ') {
		while(*str == ' ')
			str++;
	}

	return str;
}

int8_t str_is(__idata char *str, const __code char *cmd)
{
	while(*str > ' ') {
		if (*str != *cmd)
			return 0;
		str++;
		cmd++;
	}
	if (*cmd == 0)
		return 1;
	return 0;
}

uint16_t argtou(__idata char *arg, __idata char **end)
{
	uint8_t digit;
	uint16_t val = 0;

	if ((arg[0] == '0') && (arg[1] == 'x'))
		arg++;
	if (*arg == 'x') {
		arg++;
		while (*arg > ' ') {
			digit = *arg;
			if ((digit >= 'A') && (digit <= 'F'))
				digit = digit - 'A' + 10;
			else if ((digit >= 'a') && (digit <= 'f'))
				digit = digit - 'a' + 10;
			else if ((digit >= '0') && (digit <= '9'))
				digit = digit - '0';
			else
				goto retval;
			val = (val << 4) | digit;
			arg++;
		}
	} else {
		while (*arg > ' ') {
			digit = *arg;
			if ((digit >= '0') && (digit <= '9'))
				digit = digit - '0';
			else
				break;
			val = val * 10 + digit;
			arg++;
		}
	}

retval:
	while (*arg && *arg <= ' ')
		arg++;
	*end = arg;
	return val;
}

cli_processor *parser;

static uint8_t  cursor;
static __idata char cmd[CMD_LEN + 1];
static __xdata char hist[CMD_LEN + 1];

void cli_init(cli_processor *process)
{
	cursor = 0;
	cmd[0] = '\0';
	hist[0] = '\0';
	parser = process;
	uart_putsc("> ");
}

int8_t cli_exec(const __code char *str)
{
	uint8_t i, ch;
	/* copy command from code memory to __idata buffer */
	for(i = 0; i < CMD_LEN; i++) {
		ch = *str++;
		cmd[i] = ch;
		if ((ch == 0) || (ch == '\n')) {
			i = ch;
			break;
		}
	}
	ch = parser(cmd);
	if (i)
		uart_putsc("> ");
	cmd[0] = '\0'; /* reset command buffer */
	return ch;
}

int8_t cli_interact(char c)
{
	register char ch = c;
	if ((ch = terminal_getc(ch)) == 0)
		return 0;

	if (ch & EXTRA_KEY) {
		if (ch == ARROW_UP && (cursor == 0)) {
			/* execute last successful command */
			for (cursor = 0; ; cursor++) {
				ch = hist[cursor];
				cmd[cursor] = ch;
				if (ch == '\0')
					break;
				uart_putc(ch);
			}
		}
		return 1;
	}

	if (ch == '\n') {
		uart_putc(ch);
		if (*cmd) {
			cmd[cursor] = '\0';
			int8_t ret = parser(cmd);
			if (ret == CLI_EARG)
				uart_putsc("Invalid argument\n");
			else if (ret == CLI_ENOTSUP)
				uart_putsc("Unknown command\n");
			else if (ret == CLI_ENODEV)
				uart_putsc("Device error\n");
			if (ret != CLI_ENOHIST) {
				for (ch = 0; ch < cursor; ch++)
					hist[ch] = cmd[ch];
				hist[cursor] = '\0';
			}
			for (ch = 0; ch < cursor; ch++)
				cmd[ch] = '\0';
		}
		cursor = 0;
		uart_putsc("> ");
		return 1;
	}

	/* backspace processing */
	if (ch == '\b') {
		if (cursor) {
			cursor--;
			cmd[cursor] = '\0';
			uart_putsc("\b \b");
		}
	}

	/* skip control or damaged bytes */
	if (ch < ' ')
		return 0;

	/* echo */
	uart_putc(ch);

	cmd[cursor++] = ch;
	cursor &= CMD_LEN - 1;
	// clean up in case of overflow (command too long)
	if (!cursor) {
		for (uint8_t i = 0; i <= CMD_LEN; i++)
			cmd[i] = '\0';
	}

	return 1;
}
