/*
  The MIT License (MIT)

  Simple I2C Atmel 24Cxx (up to 24C512) EEPROM support for N76E003
*/

#include <N76E003.h>
#include <i2c.h>
#include <tick.h>
#include <uart.h>

#include "i2c_mem.h"

#define I2C_MEM ((uint8_t)(I2C_MEM_ADDR << 1)) /** depends on the address jumpers */

#define I2C_MEM_TIMEOUT 10 /** usually 5 msec is fine, but double it just in case */

#if I2C_MEM_CALLBACK
static i2cmem_idle_callback *pidle;

void i2cmem_set_idle_callback(i2cmem_idle_callback *pcall)
{
	pidle = pcall;
	return;
}
#endif

/** long poll for an ack in case if 24C* is busy updating memory page */
static int8_t i2cmem_ack_poll(uint8_t op, uint8_t tout)
{
	uint8_t ms = millis8();
	do {
#if I2C_MEM_CALLBACK
		if (pidle)
			pidle();
#endif
		if (i2c_start(I2C_MEM | op) == I2C_EOK)
			return I2C_EOK;
		i2c_stop();
	} while (elapsed(ms) < tout);

	return I2C_ESTART;
}

int8_t i2cmem_read_data(uint16_t addr, __idata void *dest, uint8_t len)
{
	if (!len || i2cmem_ack_poll(I2C_WRITE, I2C_MEM_TIMEOUT) != I2C_EOK)
		return I2C_ESTART;
	len--;
	i2c_write(HIBYTE(addr));
	i2c_write(LOBYTE(addr));
	i2c_start(I2C_MEM | I2C_READ);
	uint8_t *data = dest;
	for(uint8_t i = 0; i <= len; i++)
		data[i] = i2c_read(len - i);
	i2c_stop();

	return I2C_EOK;
}

int8_t i2cmem_read_xdata(uint16_t addr, __xdata void *dest, uint8_t len)
{
	if (!len || i2cmem_ack_poll(I2C_WRITE, I2C_MEM_TIMEOUT) != I2C_EOK)
		return I2C_ESTART;
	len--;
	i2c_write(HIBYTE(addr));
	i2c_write(LOBYTE(addr));
	i2c_start(I2C_MEM | I2C_READ);
	for(uint8_t data, i = 0; i <= len; i++) {
		data = i2c_read(len - i);
		((__xdata uint8_t *)dest)[i] = data;
	}
	i2c_stop();

	return I2C_EOK;
}

int8_t i2cmem_write_byte(uint16_t addr, uint8_t data)
{
	if (i2cmem_ack_poll(I2C_WRITE, I2C_MEM_TIMEOUT) != I2C_EOK)
		return I2C_ESTART;
	i2c_write(HIBYTE(addr));
	i2c_write(LOBYTE(addr));
	i2c_write(data);
	i2c_stop();

	return I2C_EOK;
}

int8_t i2cmem_write_data(uint16_t addr, __idata uint8_t *src, uint8_t len)
{
	if (!len)
		return I2C_EOK;

	if ((addr >= I2C_MEM_SIZE) || ((addr + len) > I2C_MEM_SIZE))
		return I2C_EARG;

	uint8_t offset = addr & (I2C_MEM_PAGE_SIZE - 1);
	addr &= ~(I2C_MEM_PAGE_SIZE - 1);

	do {
		uint8_t n = I2C_MEM_PAGE_SIZE - offset;
		if (n > len)
			n = len;

		if (i2cmem_ack_poll(I2C_WRITE, I2C_MEM_TIMEOUT) != I2C_EOK)
			return I2C_ESTART;
		i2c_write(HIBYTE(addr + offset));
		i2c_write(LOBYTE(addr + offset));
		for (uint8_t i = 0; i < n; i++)
			i2c_write(src[i]);
		i2c_stop();

		len -= n;
		src += n;
		offset = 0;
		addr += I2C_MEM_PAGE_SIZE;
	} while(len);

	/* idle ack check to make sure that write is done before return */
	i2cmem_ack_poll(I2C_WRITE, I2C_MEM_TIMEOUT);
	i2c_stop();

	return I2C_EOK;
}
