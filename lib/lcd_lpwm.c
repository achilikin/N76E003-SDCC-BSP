/*
  The MIT License (MIT)

  Simple driver for LCD installed on XY-LPWM and some others
  devices driven by N76E003.
*/
#include <N76E003.h>

#include "ht1621.h"
#include "lcd_lpwm.h"
#include "uart.h"

#define LCD_BUF_SIZE (LCD_NUM_SEGMENTS / 2)

static __xdata uint8_t lcd_buf[LCD_BUF_SIZE];

void lcd_buf_init(uint8_t fill)
{
	for (uint8_t i = 0; i < LCD_BUF_SIZE; i++) {
		lcd_buf[i] = fill;
		ht1621_write_data(i*2, fill);
	}
}

/**
 * @param fill character to fill memory with
 */
void lcd_init(uint8_t fill)
{
	ht1621_init(HT1621_BIAS_2 | HT1621_COM_4);
	lcd_buf_init(fill);
}

void _set_sign(uint8_t sign, bool set)
{
	uint8_t idx = sign >> 4;
	uint8_t data = lcd_buf[idx];
	uint8_t mask = 1;

	mask <<= sign & 0x0F;
	if (set)
		data |= mask;
	else
		data &= ~mask;
	lcd_buf[idx] = data;
	ht1621_write_data(idx * 2, data);
}

/**
 * signs:
 *   '0', '1', '2', '3', '4', '5', '6', '7': dots indexes
 *   ':': colon (same as '7')
 *   'I': IN
 *   'O': OUT
 *   'S': SET
 *   'C': C
 *   'V': V (same as '3')
 *   'W': W
 *   '%': %
 *   'A': A
 *   'h': h
 */

static const uint8_t lcd_signs[] = {
	LCD_SIG_DOT0, 	 0x77,  LCD_SIG_DOT1, 	 0x67,
	LCD_SIG_DOT2, 	 0x57,  LCD_SIG_DOT3, 	 0x47,
	LCD_SIG_DOT4, 	 0x07,	LCD_SIG_DOT5, 	 0x17,
	LCD_SIG_DOT6, 	 0x27,	LCD_SIG_DOT7, 	 0x37,
	LCD_SIG_V, 		 0x47,  LCD_SIG_COLON, 	 0x37,
	LCD_SIG_IN, 	 0x84,	LCD_SIG_OUT, 	 0x85,
	LCD_SIG_CGRAD, 	 0x87,	LCD_SIG_W, 		 0x40,
	LCD_SIG_PERCENT, 0x41,	LCD_SIG_A,		 0x43,
	LCD_SIG_h,		 0x42,	LCD_SIG_SET, 	 0x86,
};

void lcd_set_sign(uint8_t sign, bool set)
{
	for (uint8_t idx = 0; idx < sizeof(lcd_signs); idx += 2) {
		if (lcd_signs[idx] == sign) {
			_set_sign(lcd_signs[++idx], set);
			return;
		}
	}
	return;
}

void lcd_set_dot(uint8_t idx, bool set)
{
	if (idx < LCD_NUM_DIGITS)
		lcd_set_sign('0' + idx, set);
}

/* map segment addresses for the digits */
static const uint8_t digit_addr[LCD_NUM_DIGITS] = {
	15, 13, 11, 9, 0, 2, 4, 6
};

/**
 * get one data byte starting from the given segment index/
 * for odd segments the data is split between two bytes
 * @param seg start map segment for a digit
 */
static uint8_t lcd_get_digit(uint8_t seg)
{
	if (!(seg & 0x01))
		return lcd_buf[seg >> 1];
	seg >>= 1;
	uint8_t data = lcd_buf[seg];
	data &= 0xF0;
	data |= lcd_buf[seg + 1] & 0x0F;
	return data;
}

/* display digit but preserve corresponding 'dot' value */
static void _set_digit(uint8_t idx, uint8_t val)
{
	uint8_t data;
	idx = digit_addr[idx]; /* get digit's start map segment index */
	data = lcd_get_digit(idx); /* get buffer data for this segment */
	data &= 0x80; /* keep the highest bit unchanged - it stores 'dot' for the digit */
	data |= val;
	if (idx > 6)
		data = swap8(data);
	ht1621_write_data(idx, data);

	/* idx is a segment address */
	/* for line 0 we need to translate it to bytes */
	idx >>= 1; /* byte index in the buffer */
	data = lcd_buf[idx];
	if (idx > 3) {
		data &= 0x8F;
		data |= val & 0x70;
		lcd_buf[idx] = data;
		idx += 1;
		data = lcd_buf[idx];
		data &= 0xF0;
		data |= val & 0x0F;
	} else {
		data &= 0x80;
		data |= val;
	}
	lcd_buf[idx] = data;
	return;
}

/* hexadecimal digits */
static const uint8_t lcd_digit[16] = {
	0x5F, 0x50, 0x3D, 0x79, /* 0x0 to 0x3 */
	0x72, 0x6B, 0x6F, 0x51, /* 0x4 to 0x7 */
	0x7F, 0x7B, 0x77, 0x6E, /* 0x8 to 0xB */
	0x0F, 0x7C, 0x2F, 0x27	/* 0xC to 0xF */
};

/* supported characters defined in pairs and terminated by 0 */
static const uint8_t lcd_alpha[] = {
	'0', 0x5F, '1', 0x50, '2', 0x3D, '3', 0x79, /* 0x0 to 0x4 */
	'4', 0x72, '5', 0x6B, '6', 0x6F, '7', 0x51, /* 0x5 to 0x8 */
	'8', 0x7F, '9', 0x7B, 'A', 0x77, 'B', 0x6E, /* 0x9 to 0xC */
	'C', 0x0F, 'D', 0x7C, 'E', 0x2F, 'F', 0x27, /* 0xD to 0xF */
	'G', 0x4F, 'H', 0x76, 'I', 0x06, 'J', 0x5C,
	'L', 0x0E, 'O', 0x5F, 'P', 0x37, 'R', 0x07,
	'S', 0x6B, 'T', 0x2E, 'U', 0x5E, 'Y', 0x7A,

	'a', 0x77, 'b', 0x6E, 'c', 0x2C, 'd', 0x7C,
	'e', 0x2F, 'f', 0x27, 'g', 0x4F, 'h', 0x66,
	'i', 0x04, 'j', 0x48, 'l', 0x0C, 'm', 0x65,
	'n', 0x64, 'o', 0x6C, 'p', 0x37, 'r', 0x24,
	's', 0x6B, 't', 0x2E, 'u', 0x4C, 'y', 0x7a,

	'-', 0x20, '=', 0x28, '>', 0x26, '<', 0x70,
	'?', 0x35, '!', 0x06, '"', 0x12, '#', 0x29,
	' ', 0x00, '_', 0x08, '^', 0x01, '\'', 0x10,
	',', 0x40, '/', 0x34, '\\', 0x62,
	0,
};

void lcd_set_symbol(uint8_t idx, uint8_t val)
{
	uint8_t data;
	if (idx >= LCD_NUM_DIGITS)
		return;
	if (val < 16) {
		_set_digit(idx, lcd_digit[val]);
		return;
	}
	for (data = 0; lcd_alpha[data] != 0; data+=2)	{
		if (lcd_alpha[data] == val)	{
			_set_digit(idx, lcd_alpha[++data]);
			return;
		}
	}
	return;
}

void lcd_printn(uint16_t num, uint8_t dstart, uint8_t width)
{
	uint8_t n;
	uint16_t div = 1;

	if (width > 4) width = 4;
	for (n = 1; n < width; n++)
		div *= 10;

	if (num > 999) {
		n = 4;
		if (num > 9999)
			num = 9999;
	} else if (num > 99)
		n = 3;
	else if (num > 9)
		n = 2;
	else
		n = 1;

	for (; width; dstart++) {
		if (width > n)
			lcd_set_symbol(dstart & 0x07, ' ');
		else
			lcd_set_digit(dstart & 0x07, num / div);
		num %= div;
		div /= 10;
		width--;
	}
}

#ifdef LCD_DEBUG
void lcd_dump(void)
{
	for (uint8_t i = 0; i < LCD_BUF_SIZE; i++) {
		uart_putc(' ');
		uart_puth(lcd_buf[i]);
	}
	uart_putc('\n');
}

void lcd_set_raw(uint8_t idx, uint8_t val)
{
	if (idx >= LCD_NUM_DIGITS)
		return;
	idx = digit_addr[idx];	   /* get digit's start map segment index */
	if (idx > 6)
		val = swap8(val);
	ht1621_write_data(idx, val);

	/* idx is a segment address */
	/* for line 0 we need to translate it to bytes */
	idx >>= 1; /* byte index in the buffer */
	uint8_t data = lcd_buf[idx];
	if (idx > 3) {
		data &= 0x0F;
		data |= val & 0xF0;
		lcd_buf[idx] = data;
		idx += 1;
		data = lcd_buf[idx];
		data &= 0xF0;
		data |= val & 0x0F;
	} else {
		data = val;
	}
	lcd_buf[idx] = data;
	return;
}
#endif