/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for DS3231N RTC using buffered access
*/
#include <N76E003.h>

#include "i2c.h"
#include "ds3231.h"

__xdata uint8_t ds3231[DS3231_BUF_SIZE];

int8_t ds3231_rw(uint8_t addr, uint8_t len, uint8_t op)
{
	int8_t ret = I2C_ESTART;
	if (i2c_start(I2C_DS3231 | I2C_WRITE) == I2C_EOK) {
		i2c_write(addr);
		len += addr - 1;
		if (op == I2C_READ) {
			i2c_start(I2C_DS3231 | I2C_READ);
			for (; addr <= len; addr++)
				ds3231[addr] = i2c_read(addr < len);
		} else {
			for (; addr <= len; addr++)
				i2c_write(ds3231[addr]);
		}
		ret = I2C_EOK;
	}
	if (i2c_stop() != I2C_EOK)
		return I2C_ESTOP;
	return ret;
}

int8_t ds3231_32k_enable(bool enable)
{
	if (enable)
		ds3231[DS3231_REG_STATUS] |= DS3231_EN32KHZ;
	else
		ds3231[DS3231_REG_STATUS] &= ~DS3231_EN32KHZ;
	return ds3231_write(DS3231_REG_STATUS, 1);
}

int8_t ds3231_sqw_enable(bool enable)
{
	if (enable)
		ds3231[DS3231_REG_CTL] &= ~DS3231_INTCN;
	else
		ds3231[DS3231_REG_CTL] |= DS3231_INTCN;
	return ds3231_write(DS3231_REG_CTL, 1);
}

int8_t ds3231_sqw_freq(uint8_t freq)
{
	uint8_t data = ds3231[DS3231_REG_CTL];
	data &= ~DS3231_SQW_MASK;
	data |= freq & DS3231_SQW_MASK;
	ds3231[DS3231_REG_CTL] = data;
	return ds3231_write(DS3231_REG_CTL, 1);
}

int8_t ds3231_init(void)
{
	ds3231_rw(0, DS3231_BUF_SIZE, I2C_READ);
	/* disable 32kHz output */
	ds3231_32k_enable(false);

	/* disable SQW output, set to 8kHz default */
	uint8_t data = ds3231[DS3231_REG_CTL];
	data &= ~DS3231_SQW_MASK;
	/* disable sqw out - set to alarm instead, and set 8kHz wave */
	data |= DS3231_INTCN | DS3231_SQW8KHZ;
	ds3231[DS3231_REG_CTL] = data;
	return ds3231_write(DS3231_REG_CTL, 1);
}
