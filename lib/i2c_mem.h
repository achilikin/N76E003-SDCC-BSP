/*
  The MIT License (MIT)

  Simple I2C 24Cxx EEPROM support for N76E003.

  Default defines:
  	#define I2C_MEM_ADDR 0x52       -> I2C address
	#define I2C_MEM_SIZE (4 * 1024) -> size in bytes
	#define I2C_MEM_PAGE_SIZE 32    -> page size
*/
#ifndef N76E003_I2C_EEPROM_H
#define N76E003_I2C_EEPROM_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* default values for 24C32: Address 0x52, 4096 bytes, 32 bytes per page */
#ifndef I2C_MEM_ADDR
#define I2C_MEM_ADDR 0x52
#endif

#ifndef I2C_MEM_SIZE
#define I2C_MEM_SIZE (4 * 1024)
#endif

#ifndef I2C_MEM_PAGE_SIZE
#define I2C_MEM_PAGE_SIZE 32
#endif

#define I2C_MEM_MAX_PAGE (I2C_MEM_SIZE / I2C_MEM_PAGE_SIZE)

#ifndef I2C_MEM_CALLBACK
#define I2C_MEM_CALLBACK 0 /** set to true if call back support is needed */
#endif

#if I2C_MEM_CALLBACK
typedef void i2cmem_idle_callback(void);

/** set callback to use while waiting for long write operations */
void i2cmem_set_idle_callback(i2cmem_idle_callback *pcall);
#endif

int8_t i2cmem_reset(void); /* TODO: implement sw reset as described in the datasheet */

int8_t i2cmem_read_data(uint16_t addr, __idata void *dest, uint8_t len);
int8_t i2cmem_read_xdata(uint16_t addr, __xdata void *dest, uint8_t len);

int8_t i2cmem_write_byte(uint16_t addr, uint8_t data);
int8_t i2cmem_write_data(uint16_t addr, __idata uint8_t *src, uint8_t len);

#ifdef __cplusplus
}
#endif
#endif


