/*
  The MIT License (MIT)

  Command line processing

  Configuration defines (can be changed in Makefile):
    #define CMD_LEN 0x20 // must be power of two
*/
#ifndef N76E003_TERMINAL_H
#define N76E003_TERMINAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CMD_LEN must be power of two. Additional buffer will be allocated
 * in xdata to store the last command
 */
#ifndef CMD_LEN
#define CMD_LEN 0x20
#endif

#define CLI_EOK      0 /** success */
#define CLI_EARG    -1 /** invalid argument */
#define CLI_ENOTSUP -2 /** command not supported */
#define CLI_ENODEV  -3 /** reserved for device communication error */
#define CLI_ENOHIST -4 /** do not store command in history buffer */

/** command line processing, returns CLI_E* above */
typedef int8_t cli_processor(__idata char *buf);

void cli_init(cli_processor *process);

int8_t cli_interact(char ch);
int8_t cli_exec(const __code char *cmd);

/* helper functions */
__idata char *get_arg(__idata char *str);
int8_t str_is(__idata char *str, const __code char *cmd);
uint16_t argtou(__idata char *arg, __idata char **end);

#define EXTRA_KEY   0x80 /** control key flag */

/* support for arrow keys for very simple one command deep history */
#define ARROW_UP    (EXTRA_KEY | 0x41)
#define ARROW_DOWN  (EXTRA_KEY | 0x42)
#define ARROW_RIGHT (EXTRA_KEY | 0x43)
#define ARROW_LEFT  (EXTRA_KEY | 0x44)

#define KEY_HOME    (EXTRA_KEY | 0x01)
#define KEY_INS	    (EXTRA_KEY | 0x02)
#define KEY_DEL	    (EXTRA_KEY | 0x03)
#define KEY_END	    (EXTRA_KEY | 0x04)
#define KEY_PGUP    (EXTRA_KEY | 0x05)
#define KEY_PGDN    (EXTRA_KEY | 0x06)

/** filter and convert escape sequences to a single byte */
uint8_t terminal_getc(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif
