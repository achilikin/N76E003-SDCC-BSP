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
  will need to connect it to serial interface first and setup I2C address 0x62
    esc[98a
*/
#include <N76E003.h>
#include <i2c.h>
#include <tick.h>

#include "bv4618.h"

int8_t bv4618_cmd(uint8_t cmd)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_BV4618 | I2C_WRITE) == I2C_EOK) {
		i2c_write(0x1b);
		i2c_write(cmd);
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

int8_t bv4618_cmd_arg(uint8_t cmd, uint8_t arg)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_BV4618 | I2C_WRITE) == I2C_EOK) {
		i2c_write(0x1b);
		i2c_write(cmd);
		i2c_write(arg);
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

int8_t bv4618_goto(uint8_t line, uint8_t column)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_BV4618 | I2C_WRITE) == I2C_EOK) {
		i2c_write(0x1b);
		i2c_write(0x24);
		i2c_write(line + 1);
		i2c_write(column + 1);
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

int8_t bv4618_putc(uint8_t ch)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_BV4618 | I2C_WRITE) == I2C_EOK) {
		i2c_write(ch);
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

int8_t bv4618_puts(__idata const char *str)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_BV4618 | I2C_WRITE) == I2C_EOK) {
		uint8_t ch = *str++;
		while (ch) {
			i2c_write(ch);
			ch = *str++;
		}
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

int8_t bv4618_putsc(__code const char *str)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_BV4618 | I2C_WRITE) == I2C_EOK) {
		uint8_t ch = *str++;
		while (ch) {
			i2c_write(ch);
			ch = *str++;
		}
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

/** print uint8_t in hex format */
int8_t bv4618_puth(uint8_t val)
{
	uint8_t hex = val >> 4;
	hex += '0';
	if (hex > '9')
		hex += 7;
	bv4618_putc(hex);
	hex = val & 0x0F;
	hex += '0';
	if (hex > '9')
		hex += 7;
	return bv4618_putc(hex);
}

/** print uint16_t in dec format */
int8_t bv4618_putn(uint16_t val)
{
	if (val == 0)
		return bv4618_putc('0');

	uint8_t print = 0;
	uint16_t div = 10000;
	for (uint8_t i = 0; i < 5; i++) {
		uint8_t byte = val / div;
		if (byte)
			print++;
		if (print)
			bv4618_putc(byte + '0');
		val -= byte * div;
		div /= 10;
	}
	return 0;
}