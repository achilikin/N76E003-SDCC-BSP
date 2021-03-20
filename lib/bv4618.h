/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for legacy BV4618 LCD controller
  with some basic I2C commands support.

  For full list of available commands including serial port please
  refer to:
    http://www.byvac.com/index.php/BV4618
    http://www.byvac.co.uk/downloads/datasheets/BV4618%20Datasheet.pdf
    http://www.byvac.co.uk/downloads/BV4618/BV4618%20User%20Guide.pdf

  If you still have some of these BV4618 controllers around you probably
  will need to connect it to serial interface and setup I2C address 0x62
    esc[98a
*/
#ifndef N76E003_BV4618_H
#define N76E003_BV4618_H

#include <N76E003.h>

#ifdef __cplusplus
extern "C" {
#endif


/* BV4618 I2C address */
#define I2C_BV4618 ((uint8_t)(0x31 << 1))

int8_t bv4618_cmd(uint8_t cmd);
int8_t bv4618_cmd_arg(uint8_t cmd, uint8_t arg);

/* configuration commands */
#define bv4618_reset() bv4618_cmd(0x43)
#define bv4618_set_lines(lines) bv4618_cmd_arg(0x30,lines)
#define bv4618_set_columns(columns) bv4618_cmd_arg(0x31,columns)

/* set of clear commands */
#define bv4618_cls()     bv4618_cmd(0x50) /** clear screen */
#define bv4618_clright() bv4618_cmd(0x51) /** clear line from cursor right */
#define bv4618_clleft()  bv4618_cmd(0x52) /** clear line from cursor left */
#define bv4618_clline()  bv4618_cmd(0x53) /** clear entire line */

/* new line + carriage return */
#define bv4618_nl() bv4618_cmd(0x0a)
#define bv4618_lf() bv4618_cmd(0x0a)
#define bv4618_cr() bv4618_cmd(0x0d)

/** turn backlight led on/off */
#define bv4618_led(x) bv4618_cmd_arg(0x03,x)

/** turn scrolling on/off */
#define bv4618_disable_scroll(x) bv4618_cmd_arg(0x45,x)

/* cursor movements and control */
#define bv4618_up()    bv4618_cmd(0x20)
#define bv4618_down()  bv4618_cmd(0x21)
#define bv4618_right() bv4618_cmd(0x22)
#define bv4618_left()  bv4618_cmd(0x23)
#define bv4618_home()  bv4618_cmd(0x25)
/** line and pos are 0 based */
int8_t bv4618_goto(uint8_t line, uint8_t column);
#define bv4618_line(line) bv4618_goto(line,0)
#define bv4618_hide() bv4618_cmd_arg(0x01,0x0c)
#define bv4618_show() bv4618_cmd_arg(0x01,0x0e)
#define bv4618_blink() bv4618_cmd_arg(0x01,0x0d)

/* print on screen */
int8_t bv4618_putc(uint8_t ch); /** print one character */
int8_t bv4618_puts(__idata const char *str); /** print string from data segment */
int8_t bv4618_putsc(__code const char *str); /** print string from code segment */
int8_t bv4618_puth(uint8_t val);			 /** print uint8_t in hex format */
int8_t bv4618_putn(uint16_t val);			 /** print uint16_t in dec format */

#endif