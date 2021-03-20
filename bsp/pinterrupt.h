/*
  The MIT License (MIT)

  Simple routines to manipulate N76E003 pin inteerput configuration.
*/
#ifndef N76E003_PINTERRUPT_H
#define N76E003_PINTERRUPT_H

#include <N76E003.h>
#include <irq.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	PIN_IRQ_PORT0 = 0,
	PIN_IRQ_PORT1,
	PIN_IRQ_PORT2,
	PIN_IRQ_PORT3
};

/**
 * @param port to use to handle interrupts, index 0 to 3
 */
void pin_irq_init_port(uint8_t port);

/** pin IRQ on edge, mutually exclusive with level interrupt */
#define PIN_IRQ_EDGE 	0x80
#define PIN_IRQ_FALL 	0x01
#define PIN_IRQ_RISE	0x02

/** pin IRQ on level, mutually exclusive with edge interrupt */
#define PIN_IRQ_LEVEL	0x00
#define PIN_IRQ_LOW 	PIN_IRQ_FALL
#define PIN_IRQ_HIGH 	PIN_IRQ_RISE

#define PIN_IRQ_DISABLE 0xFF

/**
 * @param pin index 0 to 7
 * @param mode PIN_IRQ_* above
*/
void pin_irq_set_pin(uint8_t pin, uint8_t mode);

/** just a placeholder declarion, application should implement this handler */
void pin_interrupt_handler(void) INTERRUPT(IRQ_PIN, IRQ_PIN_REG_BANK);

#ifdef __cplusplus
}
#endif
#endif