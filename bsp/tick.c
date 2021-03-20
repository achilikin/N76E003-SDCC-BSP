/*
  The MIT License (MIT)

  Use N76E003 WKT interrupt to provide milliseconds counters

  Configuration defines (can be changed in Makefile):
    #define TICK_DEBUG P12 // define pit to toggle on WKT interrupt
*/
#include <N76E003.h>

#include "tick.h"
#include "event.h"

tick_t ticks;
static uint8_t evt_counter;
static uint8_t evt_interval;

void tick_interrupt_handler(void) INTERRUPT(IRQ_TICK,IRQ_TICK_REG_BANK)
{
	ticks.millis++;
	evt_counter++;

	if (evt_counter == evt_interval) {
		evt_counter = 0;
		event_put(EVT_TICK, evt_interval);
	}

#ifdef TICK_DEBUG
	TICK_DEBUG ^= 1;
#endif
	WKCON &= ~WKCON_WKTF; /* clear WKT overflow interrupt flag */
}

void tick_init(uint8_t evt_timer)
{
	/* initialize WKT, RC driven clock close to 10kHz (±10%) */
	/* POR for WKCON_WKPS is 0: pre-scale = 1/1 */
	RWK = (uint8_t)(255 - 10 + 1);	/* roll over every 4 clocks or roughly 1 millisecond */
	EIE1 |= EIE1_EWKT;			   	/* enable WKT interrupt */
	WKCON |= WKCON_WKTR; 			/* run wake-up timer */
	evt_interval = evt_timer;
	return;
}

uint8_t tick_interval(uint8_t evt_timer)
{
	uint8_t ret = evt_interval;
	cli();
	evt_interval = evt_timer;
	evt_counter = 0;
	sti();
	return ret;
}

void wait(uint16_t msec, enum POWER_MODE mode)
{
	uint16_t start;
	cli();
	start = ticks.milli16;
	sti();
	do {
		PCON &= ~(PCON_PD | PCON_IDL);
		PCON |= mode; /* go to idle/down mode, wake up on the next WKT interrupt */
		cli();
		uint16_t tsc = ticks.milli16;
		sti();
		tsc -= start;
		if (tsc >= msec)
			return;
	} while (1);
}

uint16_t millis(void)
{
	uint16_t msec;
	cli();
	msec = ticks.milli16;
	sti();
	return msec;
}

uint32_t millis32(void)
{
	uint32_t msec;
	cli();
	msec = ticks.millis;
	sti();
	return msec;
}

void delay_mks(uint16_t mks)
{
	while(mks)
		mks--;
	return;
}