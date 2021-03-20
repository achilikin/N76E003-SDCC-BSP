/*
  The MIT License (MIT)

  Simple driver for keys (push buttons) connected to pull-up pins
  so key down detected as low level
*/
#include <N76E003.h>

#include <tick.h>
#include <event.h>

#include "key.h"

/* TODO: implement virtual key as combination of multiple keys */

/** tracking state of pins assigned to the keys */
static uint8_t  kmask;		/**< bitmask of all valid keys: 1 VALID, 0 DO NOT CARE */
static uint8_t  kpins;		/**< current bitmask of the keys: 1 UP, 0 DOWN */
static uint8_t  kdown;		/**< index of a key which is down */
static uint8_t  kstate;		/**< current state of the key being tracked */
static uint8_t  krepeat;	/**< EVT_KEY_REPEAT event counter */
static uint16_t kdown_ts;	/**< timestamp of EVT_KEY_DOWN event */
static uint16_t kpress_ts;	/**< timestamp of the last EVT_KEY_PRESS event */

/**
 * enum KEY_STATE
 * KDOWN:
 *	EVT_KEY_DOWN detected
 *	kdown (key index) and timestamp initialized
 *	EVT_KEY_DOWN generated with evt::data set to kdown
 * KREADY:
 *	key is KDOWN state for KEY_READY_TIME
 *	EVT_KEY_READY generated
 * KUP = 0:
 *	EVT_KEY_UP detected
 *  if current state is KREADY then EVT_KEY_PRESS generated first
 *	EVT_KEY_UP generated
 *	if kstate is not KREADY, evt::data set to 0, else set to kdown
 * KPRESS:
 *  EVT_KEY_PRESS sent
 * KREPEAT:
 *	if key down interval == KEY_LONG_PRESS_TIME
 *	EVT_KEY_REPEAT generated
 *	EVT_KEY_PRESS generated at KEY_REPEAT_TIME interval
 */

enum KEY_STATE {
	KUP = 0,
	KDOWN,
	KREADY,
	KPRESS,
	KREPEAT,
};

void key_init(uint8_t pin_mask)
{
	kmask = kpins = pin_mask;
}

uint8_t key_is_pressed(uint8_t key_mask)
{
	return !(kpins & key_mask);
}

/**
 * generate key event, event::data = KEY_FREQ/KEY_DUTY
 * @param pin_mask current state of all keys pins (0 = key down)
 */
uint16_t key_event(uint8_t pin_mask)
{
	event_t key;

	/* detect and process key down/up events */
	if (kpins != pin_mask) {
		kdown_ts = millis();
		uint8_t mask = kpins ^ pin_mask;
		/* scan for the first bit set, only single key state change detected */
		key.data = 0x01;
		while (!(mask & key.data))
			key.data <<= 1;

		if (pin_mask & mask) { /* key had been released */
			if (kstate == KREADY) {
				key.type = EVT_KEY_PRESS;
				kstate = KPRESS;
			} else {
				key.type = EVT_KEY_UP;
				kstate = KUP;
			}
		} else {
			kstate = KDOWN;
			key.type = EVT_KEY_DOWN;
		}

		kpins = pin_mask;
		kdown = key.data; /* start tracking the pressed key */
		return key.evt;
	}

	if (kstate == KPRESS) {
		kstate = KUP;
		key.type = EVT_KEY_UP;
		goto set_data;
	}

	uint16_t ctime = millis();

	if (kstate == KUP) {
		if (kdown && ((ctime - kdown_ts) >= KEY_DONE_TIME)) {
			key.type = EVT_KEY_DONE;
			key.data = kdown;
			kdown = (kpins ^ kmask) & kmask;
			if (!kdown)
				return key.evt;
		}
		if ((kpins ^ kmask) & kmask) {
			kstate = KREADY;
			kdown_ts = ctime;
			kdown = (kpins ^ kmask) & kmask;
		}
		return 0;
	}

	if ((kstate == KDOWN) && ((ctime - kdown_ts) >= KEY_READY_TIME)) {
		krepeat = 0;
		kstate = KREADY;
		key.type = EVT_KEY_READY;
		kpress_ts = ctime;
		goto set_data;
	}

	if ((kstate == KREADY) && ((ctime - kdown_ts) >= KEY_LONG_PRESS_TIME)) {
		kstate = KREPEAT;
key_repeat_event:
		if (krepeat < 0xFF) /* do not overflow repeat event counter */
			krepeat += 1;
		kdown_ts = ctime;
		key.data = krepeat;
		key.type = EVT_KEY_REPEAT;
		return key.evt;
	}

	if (kstate == KREPEAT) {
		if ((ctime - kdown_ts) >= KEY_LONG_PRESS_TIME)
			goto key_repeat_event;
		if ((ctime - kpress_ts) >= KEY_REPEAT_TIME) {
			key.type = EVT_KEY_PRESS;
			kpress_ts = ctime;
			key.data = (kpins ^ kmask) & kmask;
			return key.evt;
		}
	}

	key.type = 0;

set_data:
	key.data = kdown;
	return key.evt;
}

#ifdef KEY_DEBUG
#include "uart.h"

static const __code char *key_events[] = {
	"NONE  ",
	"DOWN  ",
	"READY ",
	"PRESS ",
	"REPEAT",
	"UP    ",
	"DONE  "
};

void key_evt_debug(int8_t type, uint8_t data)
{
	uart_putn(millis());
	uart_putsc(" type ");
	uart_puth(type);
	uart_putsc(" (");
	uart_putsc(key_events[type]);
	uart_putsc(") data ");
	uart_puthl(data);
}
#endif