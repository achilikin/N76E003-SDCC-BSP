/*
  The MIT License (MIT)

  Simple driver for keys (push buttons) connected to pull-up pins
  from different I/O ports
*/
#ifndef PULLUP_PIN_KEY_H
#define PULLUP_PIN_KEY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** de-bouncing interval to switch to EVT_KEY_READY state, msec */
#define KEY_READY_TIME          5
/** interval for repeated EVT_KEY_PRESS events, msec */
#define KEY_REPEAT_TIME       100
/** interval for long press detection, msec, generates EVT_KEY_REPEAT event */
#define KEY_LONG_PRESS_TIME  1000
/**
 * interval for clen-up event after the key was released,
 * msec, generates EVT_KEY_DONE
 */
#define KEY_DONE_TIME     5000

/** key event types */
enum EVT_KEY {
	EVT_KEY_NONE,
	/** key down event detected, not de-bounced */
	EVT_KEY_DOWN,
	/** key down event debounced */
	EVT_KEY_READY,

	/**
	 * key pressed, generated on key up event after key ready state
	 * or as part of key repeat event
	 */
	EVT_KEY_PRESS,

	/**
	 * long key press detected, this even repeated every KEY_LONG_PRESS_TIME
	 * interval with evt.data incremented
	 */
	EVT_KEY_REPEAT,

	/**
	 * key released event, if this event follows right after EVT_KEY_DOWN,
	 * then key is bouncing, evt.data set to 0 and event can be ignored.
	 * Real EVT_KEY_UP follows EVT_KEY_PRESS and have key index in evt.data
	 */
	EVT_KEY_UP,
	/**
	 * clean-up event, generated after KEY_DONE_TIME of EVT_KEY_UP event
	 */
	EVT_KEY_DONE
};

void key_init(uint8_t bit_mask);

uint8_t key_is_pressed(uint8_t key_mask);

/**
 * generate key event, event::data = mask of keys
 * @param pin_mask current state of all keys pins (0 = key down)
 */
uint16_t key_event(uint8_t pin_mask);

#ifdef KEY_DEBUG
void key_evt_debug(int8_t type, uint8_t data);
#endif

#ifdef __cplusplus
}
#endif
#endif

