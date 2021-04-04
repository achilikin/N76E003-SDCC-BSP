#include <N76E003.h>

#include "i2c.h"
#include "tick.h"
#include "uart.h"

static __xdata uint8_t tout;

/**
 * Initialize I2C bus
 * @param clock I2C_CLOCK_100K or I2C_CLOCK_400K
 * @param tout timeout value in msec
 * @param alt  use laternative I2C pins
 */
void i2c_init(uint8_t clock, uint8_t timeout, uint8_t alt)
{
	if (alt) { /* alternative I2C pins */
		P02_OpenDrain_Mode;
		P16_OpenDrain_Mode;
		set_I2CPX;
	} else {
		P13_OpenDrain_Mode;
		P14_OpenDrain_Mode;
		clr_I2CPX;
	}
	tout = timeout;
	I2CLK = clock;
	I2CEN = 1;
}

static int8_t wait_si(void)
{
	uint8_t ts;

	SI = 0;
	ts = millis8();
	while (SI == 0) {
		if (elapsed(ts) >= tout)
			return I2C_EWAIT;
	}
	return I2C_EOK;
}

int8_t i2c_start(uint8_t device)
{
	uint8_t read = device & I2C_READ;

	STA = 1;
	wait_si();
	STA = 0;

	/* some sanity checks for start condition */
	if (read && (I2STAT != 0x10))
		return I2C_ESTART;
	else if (!read && (I2STAT != 0x08))
		return I2C_ESTART;

	I2DAT = device;
	if (wait_si() != I2C_EOK)
		return I2C_ESTART;

	/* check if device responded */
	if (read && (I2STAT != 0x40))
		return I2C_ESTART;
	else if (!read && (I2STAT != 0x18))
		return I2C_ESTART;

	return I2C_EOK;
}

int8_t i2c_stop(void)
{
	uint8_t ts;
	STO = 1;
	SI = 0;
	ts = millis8();
	while (STO == 1) {
		if (elapsed(ts) >= tout)
			return I2C_ESTOP;
	}

	return I2C_EOK;
}

uint8_t i2c_read(uint8_t ack)
{
	uint8_t data;

	AA = 1;
	if (wait_si())
		return 0;

	data = I2DAT;

	if (!ack) {
		AA = 0;
		wait_si();
	}

	return data;
}

int8_t i2c_write(uint8_t data)
{
	I2DAT = data;
	return wait_si();
	/* Todo: can check I2C here, 0x28 - ACK received, 0x30 - NACK received */
}
