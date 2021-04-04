/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for DHT22 (AM2302) sensor
*/
#include <N76E003.h>
#include <tick.h>
#include <uart.h>

#include "dht.h"

dht_t dht;

#ifdef DHT_DEBUG
/** print counter for bit state detection, used for DHT_ONE */
static uint8_t dht_counter;
#endif

static uint8_t dht_skip_bit(uint8_t val)
{
	uint8_t counter = 0;
	uint8_t ts = millis8();
	while (DHT_PIN == val) {
		counter++;
		if (elapsed(ts) >= DHT_TIMEOUT)
			return 0;
	}
	if (!counter)
		return 0xFF;
	return counter;
}

static uint8_t dht_read_byte(void)
{
	uint8_t val = 0;
	for (uint8_t i = 0; i < 8; i++) {
		val <<= 1;
		if (!dht_skip_bit(0))
			return 0; /* timeout */
#ifdef DHT_DEBUG
		dht_counter = dht_skip_bit(1);
		if (dht_counter > DHT_ONE)
#else
		if (dht_skip_bit(1) > DHT_ONE)
#endif
			val |= 0x01;
	}

	return val;
}

uint8_t dht_read(void)
{
	uint8_t i;

	DHT_PIN = 0;
	delay(2);
	DHT_PIN = 1;

	dht_skip_bit(1);
	if (!dht_skip_bit(0))
		return DHT_ERR_TIMEOUT;
	dht_skip_bit(1);

	for (i = 0; i < 4; i++)
		dht.raw[i] = dht_read_byte();

	uint8_t crc = dht_read_byte();

	if (!(dht.raw[0] | dht.raw[1])) {
#ifdef DHT_DEBUG
		uart_putsc("read");
		uart_putsc(" error\n");
#endif
		DHT_PIN = 1; /* make sure to go to the known state */
		return DHT_ERR_TIMEOUT;
	}

#ifdef DHT_DEBUG
	uart_putsc("crc ");
	uart_puth(crc);
#endif

	for (i = 0; i < 4; i++)
		crc -= dht.raw[i];

	if (crc) {
#ifdef DHT_DEBUG
		uart_putsc(" error\n");
#endif
		return DHT_ERR_CRC;
	}

#ifdef DHT_DEBUG
	uart_putsc(" OK");
	uart_putc(' ');
	uart_putn(dht_counter);
	uart_putc('\n');
#endif

	dht.data.rh = swap16(dht.data.rh);
	dht.data.tc = swap16(dht.data.tc);

#ifdef DHT_DEBUG
	uart_puthw(dht.data.rh);
	uart_putc(' ');
	uart_putn(dht.data.rh);
	uart_putc('\n');

	uart_puthw(dht.data.tc);
	uart_putc(' ');
	uart_putn(dht.data.tc);
	uart_putc('\n');
#endif

	return DHT_OK;
}
