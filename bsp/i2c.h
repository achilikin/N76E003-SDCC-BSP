#ifndef N76E003_I2C_H
#define N76E003_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* I2C Clock = Fsys / (4*(prescaler + 1)) */
#define I2C_CLOCK_100K (40)
#define I2C_CLOCK_400K (10)
#define I2C_CLOCK_800K (4)
#define I2C_CLOCK_1MHZ (3)

/* defines the data direction (reading/writing from I2C device) in i2c_start() */
#define I2C_READ    1
#define I2C_WRITE   0

#define I2C_ACK  1
#define I2C_NACK 0

#define I2C_EOK 	 0
#define I2C_ESTART 	-1
#define I2C_ESTOP  	-2
#define I2C_EWAIT  	-3
#define I2C_EARG 	-4

/**
 * Initialize I2C bus
 * @param clock I2C_CLOCK_100K or I2C_CLOCK_400K
 * @param tout timeout value in msec
 * @param alt  use laternative I2C pins
 */
void    i2c_init(uint8_t clock, uint8_t tout, uint8_t alt);
int8_t  i2c_start(uint8_t device);
int8_t  i2c_write(uint8_t data);
uint8_t i2c_read(uint8_t ack);
int8_t  i2c_stop(void);

#ifdef __cplusplus
}
#endif

#endif
