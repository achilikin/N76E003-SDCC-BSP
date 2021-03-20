/*
  The MIT License (MIT)

  Simple routines to manipulate N76E003 pin inteerput configuration.
*/
#include <N76E003.h>

#include "pinterrupt.h"

void pin_irq_init_port(uint8_t port)
{
	cli_pin();
	PICON = 0xFC;	/* edge triggered interrupt for all pins */
	PICON |= port & 0x03;
	PINEN = 0;		/* disable low-level/falling edge detection */
	PIPEN = 0;		/* disable high-level/rising edge detection */
	PIF = 0;		/* clear interrupts */
	sti_pin();
}

void pin_irq_set_pin(uint8_t pin, uint8_t mode)
{
	uint8_t mask = 0x80; /* pins 6 & 7 */

	pin &= 0x07;
	/* calculate mask for PICON */
	if (pin < 4) /* pin 0 to 3 mapped to 0x04 to 0x20 */
		mask = 0x04 << pin;
	else if (pin < 6) /* pins 4 & 5 */
		mask >>= 1;

	/* translate from pin number to pin bit for PIPEN & PINEN */
	pin = 1 << pin;

	cli_pin();
	if (mode == PIN_IRQ_DISABLE) {
		PICON |= mask;
		PINEN &= ~mask;
		PIPEN &= ~mask;
		return;
	}
	/* EDGE trigger is selected by setting bit in PICON */
	if (mode & PIN_IRQ_EDGE)
		PICON |= mask;
	else
		PICON &= ~mask;

	/* Rise/Fall edge selection */
	if (mode & PIN_IRQ_RISE) {
		PIPEN |= pin; /* set Pin Interrupt Positive ENabled - RISE edge */
		if (!(mode & PIN_IRQ_FALL))
			PINEN &= ~pin; /* clear Pin Interrupt Negative ENabled - FALL edge */
	}
	if (mode & PIN_IRQ_FALL) {
		PINEN |= pin; /* set Pin Interrupt Negative ENabled - FALL edge */
		if (!(mode & PIN_IRQ_RISE))
			PIPEN &= ~pin; /* clear Pin Interrupt Positive ENabled - RISE edge */
	}
	sti_pin();
	return;
}

