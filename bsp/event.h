#ifndef N76E003_EVENT_H
#define N76E003_EVENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Simple union of uint16_t and two bytes: event type and event data
 */
typedef union {
	uint16_t evt;
	uint8_t raw[2];
	struct {
		uint8_t type; /** event type */
		uint8_t data; /** event data */
	};
} event_t;

/**
 * event type can be treated as two nibles:
 * high 4 bits are event family
 * low  4 bits are event type
 * events 0x00 to 0x0F are reserved for system
 * families 0x10 to 0xF0 are for application to define
 */
enum EVENT_TYPE {
	EVT_NONE,	  /** 0 No events */
	EVT_ERROR,	  /** 1 Event overflow source */
	EVT_UART_RX,  /** 2 UART received byte */
	EVT_I2C_DAT,  /** 3 I2C I2CDAT byte */
	EVT_I2C_STAT, /** 4 I2C status byte */
	EVT_PIN_LOW,  /** 5 Pin status byte */
	EVT_PIN_HIGH, /** 6 Pin status byte */
	EVT_TICK,	  /** 7 timer event */
};

/**
 * put event to the buffer, designed to be called only from ISRs
 */
void event_put(uint8_t type, uint8_t data) __reentrant __using(IRQ_REG_BANK);

/** get event from the events buffer */
uint16_t event_get(void);

/** clear the events buffer */
void event_flush(void);

#ifdef __cplusplus
}
#endif

#endif
