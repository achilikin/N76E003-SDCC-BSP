/*
  The MIT License (MIT)

  Simple driver for LCD installed on XY-LPWM and some others
  devices driven by N76E003.
*/
#ifndef HT1621_LCD_LPWM_H
#define HT1621_LCD_LPWM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#define LCD_NUM_DIGITS   8  /** number of digits on the display */
#define LCD_NUM_SEGMENTS 18 /** size of active HT1621 segments map */

void lcd_init(uint8_t fill);
void lcd_buf_init(uint8_t fill);
#define lcd_clear() lcd_buf_init(0)
#define lcd_fill() lcd_buf_init(0xFF)

/**
 * Internal HT1621 segments map for XY-LPWM LCD
 * Digits are numerated 1 to 4 per line, so
 *  2.1A means: segment A for digit 1 on line 2.
 *  1.3dot means: dot segment for digit 3 on line 1
  * HT1621 expects segs to be written D0 firsts
 *  seg  D3    D2    D1    D0
 *  ---  ----  ----  ----  -------
 *    0  2.1A  2.1F  2.1E  2.1D
 *    1  2.1B  2.1G  2.1C  2.1dot
 *    2  2.2A  2.2F  2.2E  2.2D
 *    3  2.2B  2.2G  2.2C  2.2dot
 *    4  2.3A  2.3F  2.3E  2.3D
 *    5  2.3B  2.3G  2.3C  2.3dot
 *    6  2.4A  2.4F  2.4E  2.4D
 *    7  2.4B  2.4G  2.4C  2.colon
 *    8  2.W   2.%   2.h   2.A (signs)
 *    9  1.4B  1.4G  1.4C  1.V
 *   10  1.4A  1.4F  1.4E  1.4D
 *   11  1.3B  1.3G  1.3C  1.3dot
 *   12  1.3A  1.3F  1.3E  1.3D
 *   13  1.2B  1.2G  1.2C  1.2dot
 *   14  1.2A  1.2F  1.2E  1.2D
 *   15  1.1B  1.1G  1.1C  1.1dot
 *   16  1.1A  1.1F  1.1E  1.1D
 *   17  1.IN  1.OUT 2.SET 1.°C (signs)
 *
 * LCD segments buffer map
 *
 * digits have sequential indexing like this:
 *  0. 1. 2. 3
 *  4. 5. 6. 7
 * so 4D means: segment D for digit 4 (the first digit on the line 2)
 *  segments bits are reversed, so dot is always the highest bit
 *
 * buff seg  data  data  data  data
 * addr addr bit3  bit2  bit1  bit0
 * ---- ---- ----  ----  ----  ----
 * 0x00   0  4D    4E    4F    4A
 *        1  4dot  4C    4G    4B
 * 0x01   2  5D    5E    5F    5A
 *        3  5dot  5C    5G    5B
 * 0x02   4  6D    6E    6F    6A
 *        5  6dot  6C    6G    6B
 * 0x03   6  7D    7E    7F    7A
 *        7  colon 7C    7G    7B
 * 0x04   8  A     h     %     W (signs)
 *        9  V     3C    3G    3B
 * 0x05  10  3D    3E    3F    3A
 *       11  2dot  2C    2G    2B
 * 0x06  12  2D    2E    2F    2A
 *       13  1dot  1C    1G    1B
 * 0x07  14  1D    1E    1F    1A
 *       15  0dot  0C    0G    0B
 * 0x08  16  0D    0E    0F    0A
 *       17  °C    SET   OUT   IN  (signs)
 */

/**
 * signs can be addressed by special acronyms:
 *   '0', '1', '2', '4', '5', '6': dots indexes
 *   ':': colon
 *   'I': IN
 *   'O': OUT
 *   'S': SET
 *   'C': °C
 *   'V': V
 *   'W': W
 *   '%': %
 *   'A': A
 *   'h': h
 */
#define LCD_SIG_DOT0	'0'
#define LCD_SIG_DOT1	'1'
#define LCD_SIG_DOT2	'2'
#define LCD_SIG_DOT3	'3' /** same as LCD_SIG_V */
#define LCD_SIG_V		'V'

#define LCD_SIG_DOT4	'4'
#define LCD_SIG_DOT5	'5'
#define LCD_SIG_DOT6	'6'
#define LCD_SIG_DOT7	'7' /** same as LCD_SIG_COLON */
#define LCD_SIG_COLON	':'

#define LCD_SIG_W		'W'
#define LCD_SIG_PERCENT	'%'
#define LCD_SIG_h		'h'
#define LCD_SIG_A		'A'

#define LCD_SIG_IN		'I'
#define LCD_SIG_OUT		'O'
#define LCD_SIG_SET		'S'
#define LCD_SIG_CGRAD	'C'

/**
 * Turn given sign ON or OFF
 * @param sign to set ON/OFF
 * @param on true to set ON, false to set OFF
 */
void lcd_set_sign(uint8_t sign, bool on);

/**
 * Turn given dot sign ON or OFF
 * @param idx dot index 0-7 to set ON/OFF
 * @param on true to set ON, false to set OFF
 */
void lcd_set_dot(uint8_t idx, bool set);

/**
 * Dispaly the symbol at the given digit postion
 * @param pos index 0 to 7
 * @param val any of supported symbols
 */
void lcd_set_symbol(uint8_t pos, uint8_t val);

/**
 * Dispaly hexadecimal digit at the given postion
 * @param pos segment position index 0 to 7
 * @param val hexadecimal 0x00 to 0x0F, else - clear
 */
#define lcd_set_digit(pos,val) lcd_set_symbol(pos,val)

/**
 * Display 16bit value starting at given position
 * and with given width. Will be left-padded with spaces if needed.
 * No checks for width validity are done, so if wrong starting position
 * is provied then value will overflow to pos 0.
 * @param val value in range 0 to 9999 to display
 * @param pos starting position to display
 * @param width number of postions to be used
 */
void lcd_printn(uint16_t val, uint8_t pos, uint8_t width);

#ifdef LCD_DEBUG
/** dump lcd memory buffer buffer */
void lcd_dump(void);
/** write raw data to lcd memory buffer buffer */
void lcd_set_raw(uint8_t pos, uint8_t val);
#endif

#ifdef __cplusplus
}
#endif
#endif
