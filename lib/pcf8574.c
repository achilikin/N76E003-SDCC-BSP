/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for PCF8574 Remote 8-Bit I/O Expander for I2C Bus
  Not all functionality enabled, but can be easily added if needed

  Default configuration, re-define externally or below if needed:
  	#define I2C_PCF8574 ((uint8_t)(0x27 << 1))
	#define PCF8574_LINES 4
	#define PCF8574_CHARS 16

*/
#include <N76E003.h>
#include <i2c.h>
#include <tick.h>

#include "pcf8574.h"

/* instructions */
#define LCD_CLEARDISPLAY 	0x01 /** clears entire display and sets address to 0 */
#define LCD_RETURNHOME 		0x02 /** sets address to 0 and disables shift */
#define LCD_ENTRYMODESET 	0x04 /** sets cursor move direction */
#define LCD_DISPLAYCONTROL 	0x08 /** sets display on/off and controls cursor */
#define LCD_CURSHIFT 		0x10 /** cursor or display shift */
#define LCD_FUNCTIONSET 	0x20 /** sets interface length, lines and font */
#define LCD_SETCGRAMADDR 	0x40 /** sets CGRAM address */
#define LCD_SETDDRAMADDR 	0x80 /** sets DDRAM address */

#define LCD_PIN_E  0x04  /* Enable read/write bit */
#define LCD_PIN_RW 0x02  /* Read/Write bit, not used, we always write */
#define LCD_PIN_RS 0x01  /* instruction/data Register Select bit */

/* flags for display on/off control */
#define LCD_BLINKON   0x01 /** not used in the code below */
#define LCD_CURSORON  0x02 /** cursor on */
#define LCD_DISPLAYON 0x04 /** display active */
#define LCD_BACKLIGHT 0x08 /** backlight control bit */

/* flags for display entry mode */
#define LCD_ENTRYLEFT 			0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01

/* stores backlight on/off state */
static uint8_t backlight;
/* cursor position calculated by pcf_putc() */
static uint8_t cur_pos, cur_line;
/* display/cursor on/of flags */
static __xdata uint8_t display_ctl;

/**
 * raw I2C write of 8 bits:
 *   0x01: RS
 *   0x02: RW
 *   0x04: E
 *   0x08: LED backlight key
 *   0x10: D4
 *   0x20: D5
 *   0x40: D6
 *   0x80: D7
 */
static int8_t pcf_i2c_write(uint8_t data)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_PCF8574 | I2C_WRITE) == I2C_EOK) {
		i2c_write(data);
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

/**
 * writes 4bits of data to lcd controller in 3 steps:
 * 0: writes data with E bit set to 0
 * 1: writes data with E bit set to 1
 * 2: writes data with E bit set to 0
 *
 * @param data high four bits are mapped to D7-D4, lower four bits LED,E,RW,RS
 */
static void pcf_write_4bits(uint8_t data)
{
	/* make sure that we set backlight bit if needed */
	data |= backlight;
	pcf_i2c_write(data | LCD_PIN_E); /* set E high */
	delay_mks(0);
	pcf_i2c_write(data); /* set E low to initialize write */
	delay_mks(0);
}

void pcf_send_cmd(uint8_t value)
{
	pcf_write_4bits((value & 0xf0));
	pcf_write_4bits(((value << 4) & 0xf0));
	delay_mks(35); /* data write delay */
}

void pcf_send_data(uint8_t value)
{
	/* for data mode RS bit is set */
	pcf_write_4bits((value & 0xf0) | LCD_PIN_RS);
	pcf_write_4bits(((value << 4) & 0xf0) | LCD_PIN_RS);
	delay_mks(35); /* data write delay */
}

/* I2C clock should be set to 100K for PCF8574 to work stable */
void pcf_init(void)
{
	/* reset cursor position */
	cur_pos = cur_line = 0;
	/* set backlight ON by deafult */
	backlight = LCD_BACKLIGHT;
	/* set all PCF8574 pins to 0 (except backlight LED) */
	pcf_i2c_write(LCD_BACKLIGHT);
	delay(30);

	/* initialize 4bit mode with this 4-steps sequence */
	pcf_write_4bits(0x30);
	delay(4);
	pcf_write_4bits(0x30);
	delay(2);
	pcf_write_4bits(0x30);
	pcf_write_4bits(0x20);

	/* we can start sending commands now */
	/* 0x08 for >= 2 lines, 0x04 for 5x11 dots font */
#if (PCF8574_LINES < 2)
	pcf_send_cmd(LCD_FUNCTIONSET);
#else
	pcf_send_cmd(LCD_FUNCTIONSET | 0x08);
#endif

	/* turn the display on, turn cursor off, turn blinking off */
	display_ctl = LCD_DISPLAYON;
	pcf_send_cmd(LCD_DISPLAYCONTROL | display_ctl);

	/* clear display */
	pcf_cls();

	/* set text direction to left-to-right */
	pcf_send_cmd(LCD_ENTRYMODESET | LCD_ENTRYLEFT);

	/* restore home position */
	pcf_send_cmd(LCD_RETURNHOME);
	delay(2);

	return;
}

void pcf_led_enable(bool enable)
{
	if (enable)
		backlight = LCD_BACKLIGHT;
	else
		backlight = 0;
	pcf_i2c_write(backlight);
	return;
}

void pcf_cursor_enable(bool enable)
{
	if (enable)
		display_ctl |= LCD_CURSORON;
	else
		display_ctl &= ~LCD_CURSORON;
	pcf_send_cmd(LCD_DISPLAYCONTROL | display_ctl);
	return;
}

void pcf_cls(void)
{
	cur_pos = cur_line = 0;
	pcf_send_cmd(LCD_CLEARDISPLAY);
	delay(3);
	return;
}

void pcf_home(void)
{
	cur_pos = cur_line = 0;
	pcf_send_cmd(LCD_RETURNHOME);
	delay(3);
	return;
}

void pcf_goto(uint8_t line, uint8_t col)
{
	/**
	 * TODO: different displays have different addresses for 16 chars?
	 * by default use Ampire AC-164A addressing
	 */
#if (PCF8574_CHARS == 16)
	uint8_t row_offsets[] = {0x00, 0x40, 0x10, 0x50};
#else
	uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
#endif
	if ((line < PCF8574_LINES) && (col < PCF8574_CHARS)) {
		pcf_send_cmd(LCD_SETDDRAMADDR | (row_offsets[line] + col));
		cur_line = line;
		cur_pos = col;
	}
	return;
}

void pcf_putc(uint8_t ch)
{
	pcf_send_data(ch);
	cur_pos += 1;
	if (cur_pos == PCF8574_CHARS) {
		cur_pos = 0;
		cur_line = (cur_line + 1) & (PCF8574_LINES - 1);
		pcf_goto(cur_line, cur_pos);
	}
	return;
}

/** print string from data segment */
void pcf_puts(__idata const char *str)
{
	while(*str)
		pcf_putc(*str++);
	return;
}

/** print string from code segment */
void pcf_putsc(__code const char *str)
{
	while (*str)
		pcf_putc(*str++);
	return;
}

/** print uint8_t in hex format */
void pcf_puth(uint8_t val)
{
	uint8_t hex = val >> 4;
	hex += '0';
	if (hex > '9')
		hex += 7;
	pcf_putc(hex);
	hex = val & 0x0F;
	hex += '0';
	if (hex > '9')
		hex += 7;
	pcf_putc(hex);
	return;
}

/** print uint16_t in dec format */
void pcf_putn(uint16_t val)
{
	if (val == 0)
		return pcf_putc('0');

	uint8_t print = 0;
	uint16_t div = 10000;
	for (uint8_t i = 0; i < 5; i++) {
		uint8_t byte = val / div;
		if (byte)
			print++;
		if (print)
			pcf_putc(byte + '0');
		val -= byte * div;
		div /= 10;
	}
	return;
}

/** clear line from cursor right */
void pcf_clright(void)
{
	if (!cur_pos)
		return;
	uint8_t line = cur_line;
	uint8_t pos = cur_pos;
	/* cur_pos will be set to 0 by pcf_putc() when line is done */
	while (cur_pos != 0)
		pcf_putc(' ');
	pcf_goto(line, pos);
	return;
}

/** clear entire line */
void pcf_clline(void)
{
	pcf_goto(cur_line, 0);
	for (uint8_t i = 0; i < PCF8574_CHARS; i++)
		pcf_putc(' ');
	pcf_goto(cur_line, 0);
}
