/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for PCF8574 Remote 8-Bit I/O Expander for I2C Bus
  Not all functionality enabled, but can be easily added if needed

  Default configuration, re-define externally or below if needed:
  	#define I2C_PCF8574 ((uint8_t)(0x27 << 1))
	#define PCF8574_LINES 4
	#define PCF8574_CHARS 16

*/
#ifndef N76E003_PCF8574_H
#define N76E003_PCF8574_H
/**
 * PCF8574 Remote 8-Bit I/O Expander for I2C Bus
 * https://www.ti.com/lit/ds/symlink/pcf8574.pdf
 * https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf
 * I2C clock: 100kHz
 *
 * Hitachi HD44780 based LCD/Ampire AC-164A
 *  Hitachi https://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller
 *          https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
 *  Ampire AC-164A http://www.microchip.ua/LCD/Ampire/pdf/AC-164A.pdf
 *
 * pin signal
 *   1 Vss GND
 *   2 Vdd +5V
 *   3 Vo  R8 contrast pin
 *   4 RS  P0 0: Instruction, 1: Data
 *   5 RW  P1 0: Write, 1: Read
 *   6 E   P2 Enable data read/write
 *   7 D0
 *   8 D1
 *   9 D2
 *  10 D3
 *  11 D4  P4
 *  12 D5  P5
 *  13 D6  P6
 *  14 D7  P7
 *  15 A   JP1 LED+
 *  16 K   P3  LED- key
 *
 *  or in horizontal table format:
 *  +----+-+----+----+----+----+--+--+--+--+----+----+----+--+---+---+
 *  |  K |A| D7 | D6 | D5 | D4 |D3|D2|D1|D0| E  | RW |RS  |Vo|Vdd|GND|
 *  +----+-+----+----+----+----+--+--+--+--+----+----+----+--+---+---+
 *  |0x08| |0x80|0x40|0x20|0x10|  |  |  |  |0x04|0x02|0x01|  |   |   |
 *  +----+-+----+----+----+----+--+--+--+--+----+----+----+--+---+---+
 *
 *  Default I2C address 0x27 (TI) or 0x3F (NXP), can be changed by A0-A1 jumpers
 *  A0 A1 A2  Address
 *   |  |  |  0x20 0x38 '|': jumper on, '-': open
 *   |  |  -  0x21 0x39
 *   |  -  |  0x22 0x3A
 *   |  -  -  0x23 0x3B
 *   -  |  |  0x24 0x3C
 *   -  |  -  0x25 0x3D
 *   -  -  |  0x26 0x3E
 *   -  -  -  0x27 0x3F
 */

/* PCF8574 I2C address */
#ifndef I2C_PCF8574
#define I2C_PCF8574 ((uint8_t)(0x27 << 1))
#endif

#ifndef PCF8574_LINES
#define PCF8574_LINES 4
#endif

#ifndef PCF8574_CHARS
#define PCF8574_CHARS 16
#endif

/** initialize 4bit mode and apply default configuration */
void pcf_init(void);

/** send byte in command mode */
void pcf_send_cmd(uint8_t cmd);

/** send byte in data mode */
void pcf_send_data(uint8_t data);

/** clear screen and set cursor position to 0:0 */
void pcf_cls(void);
/** move cursor to position to 0:0 and disable scrolling (not enabled anyway) */
void pcf_home(void);
/** turn cursor on/off */
void pcf_cursor_enable(bool enable);

/** turn backlight led on/off */
void pcf_led_enable(bool enable);

/**
 * move cursor to a new postion
 * @param line LCD line index starting from 0
 * @param pos character postion within the line, starting from 0
 */
void pcf_goto(uint8_t line, uint8_t pos);
#define pcf_line(n) pcf_goto(n,0)

/**
 * clear line from cursor right, works only if cursor is not at position 0,
 * for position 0 use pcf_clrline instead
 */
void pcf_clright(void);
void pcf_clleft(void);	/** clear line from cursor left */
void pcf_clline(void);	/** clear entire line */

/** print one character and automatically go to the next line if needed */
void pcf_putc(uint8_t ch);
void pcf_puts(__idata const char *str); /** print string from data segment */
void pcf_putsc(__code const char *str); /** print string from code segment */
void pcf_puth(uint8_t val);				/** print uint8_t in hex format */
void pcf_putn(uint16_t val);			/** print uint16_t in dec format */

#endif