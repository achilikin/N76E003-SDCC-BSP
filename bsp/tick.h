/*
  The MIT License (MIT)

  Use N76E003 Wake-up Timer (WKT) interrupt to provide milliseconds counters

  Configuration defines (can be changed in Makefile):
    #define TICK_DEBUG P12 // define pit to toggle on WKT interrupt
*/
#ifndef N76E003_WKT_H
#define N76E003_WKT_H

#include <stdint.h>
#include <stdbool.h>

#include "irq.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile union {
	uint32_t millis;
	uint16_t milli16;
	uint8_t  milli8;
} wkt_tick_t;

extern wkt_tick_t wkt_ticks;

/** defines which IRQ to use - WKT or WDT, only WKT supported by now */
#define IRQ_TICK			IRQ_WKT
#define IRQ_TICK_REG_BANK	IRQ_WKT_REG_BANK

void tick_interrupt_handler(void) INTERRUPT(IRQ_TICK,IRQ_TICK_REG_BANK);

/**
 * @brief initialize 'tick' timer interrupts for 1 msec timer
 *
 * @param evt_timer interval in msec to generate EVT_TICK. 0 to disable event.
 */
void tick_init(uint8_t evt_timer);

/**
 * @brief set tick interval:
 *
 * @param evt_timer new interval for tick event. 0 to disable event.
 * @return previous interval value
 */
uint8_t tick_interval(uint8_t evt_timer);

enum POWER_MODE {
	POWER_MODE_UP = 0,
	POWER_MODE_IDLE = PCON_IDL,
	POWER_MODE_DOWN = PCON_PD
};

/**
 * @brief wait for number of timer clocks
 *
 * @param ticks number of WKT ticks to wait
 * @param mode waiting mode, POWER_MODE_UP, POWER_MODE_IDLE, POWER_MODE_DOWN
 */
void wait(uint16_t ticks, enum POWER_MODE mode);

inline uint8_t millis8(void) { return wkt_ticks.milli8; }
uint16_t millis(void);
uint32_t millis32(void);

#define delay(msec) wait(msec,POWER_MODE_UP) /** delay in milliseconds */
#define idle(msec) wait(msec,POWER_MODE_IDLE)
#define sleep(msec) wait(msec,POWER_MODE_DOWN)

/**
 * delay in microseconds, measured with oscilloscope:
 * mks real
 *   0   ~2
 *   1   ~3
 *   5   ~7
 *  10  ~12
 *  25  ~26
 *  50  ~51
 * 100  ~99
 * 250 ~245
 */
void delay_mks(uint16_t mks);

/* elapsed time */
inline uint8_t elapsed(uint8_t ts) {
	return (uint8_t)(wkt_ticks.milli8 - (uint8_t)ts);
}

#ifdef __cplusplus
}
#endif

#endif
