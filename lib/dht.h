/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for DHT22 (AM2302) sensor
*/
#ifndef N73E003_DHT_H
#define N73E003_DHT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DHT_PIN
#define DHT_PIN P05 /** pin, DHT connected to */
#endif

#define DHT_ONE		20	/** counter value for detecting '1' bit */
#define DHT_TIMEOUT 2	/** 2 msec timeout */

#define DHT_OK			0 /** success */
#define DHT_ERR_TIMEOUT	1 /** timeout, or device not present */
#define DHT_ERR_CRC		2 /** communication error */

typedef union dht_u {
	uint8_t raw[4];
	struct {
		uint16_t rh;
		uint16_t tc;
	} data;
} dht_t;

extern dht_t dht;

#define dht_init() DHT_PIN = 1
uint8_t dht_read(void);

#ifdef __cplusplus
}
#endif

#endif

