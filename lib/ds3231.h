/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for DS3231N RTC using buffered access
*/
#ifndef N76E003_DS3231N_H
#define N76E003_DS3231N_H

#include <N76E003.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"

/* DS3231 I2C address */
#define I2C_DS3231 ((uint8_t)(0x68 << 1))

/*
 DS3231 data buffer for buffered I/O.
 To get data first call read function then use data from the buffer.
 To set data first change data in the buffer and then call write.
 */
#define DS3231_BUF_SIZE 0x13
extern __xdata uint8_t ds3231[DS3231_BUF_SIZE];

/* Time registers */
#define DS3231_REG_SEC	0x00
#define DS3231_REG_MIN	0x01
#define DS3231_REG_HOUR	0x02

/* Control register 0x0E and relevant bits */
#define DS3231_REG_CTL	0x0E
#define DS3231_CONV		0x20 /** convert temperature */
#define DS3231_SQW1HZ	0x00 /** 1Hz Square Wave Output */
#define DS3231_SQW1KHZ	0x08 /** 1kHz Square Wave Output */
#define DS3231_SQW4KHZ	0x10 /** 4Hz Square Wave Output */
#define DS3231_SQW8KHZ	0x18 /** 8Hz Square Wave Output (default) */
#define DS3231_SQW_MASK 0x18 /** rate select bits */
#define DS3231_INTCN	0x04 /** Interrupt Control, 0 - SQW, 1 - Alarm */

/* Status/Control register 0x0F and relevant bits */
#define DS3231_REG_STATUS	0x0F
#define DS3231_EN32KHZ		0x08

/* Temperature registers */
#define DS3231_REG_TEMP_MSB	0x11
#define DS3231_REG_TEMP_LSB	0x12

int8_t ds3231_init(void);
int8_t ds3231_rw(uint8_t addr, uint8_t len, uint8_t op);
#define ds3231_read(addr,len) ds3231_rw(addr, len, I2C_READ)
#define ds3231_write(addr,len) ds3231_rw(addr, len, I2C_WRITE)

/** turn 32kHz output on/off */
int8_t ds3231_32k_enable(bool enable);
/** turn SQW output on/off */
int8_t ds3231_sqw_enable(bool enable);
/**
 * turn SQW output on/off
 * @param freq one of DS3231_SQW*HZ values
 */
int8_t ds3231_sqw_freq(uint8_t freq);

#define todec(val) (val - 6 * (val >> 4))
#define tobcd(val) (val + 6 * (val / 10))

#ifdef __cplusplus
}
#endif

#endif
