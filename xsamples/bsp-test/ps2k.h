/**
 * Example of pin iterrupt processing using PS/2 keyboard as interrupt source
 *
 * https://wiki.osdev.org/PS/2_Keyboard
 *
 * Commands reply usually 0xFA (ACK) or 0xFE (Resend)
 *
 * cmd data  description
 * xED led   Set LEDs: 0x01 Scroll, 0x02 Num, 0x04 Caps lock
 * xEE       Echo, reply 0xEE (Echo) or 0xFE (Resend)
 * xF0 code  Get/Set current code set. 0 - get, 1 to 3 - set scan code set 1 to 3
 * xF2       Identify keyboard
 * xF4       Enable scanning (keyboard will send scan codes)
 * xF5       Disable scanning (keyboard won't send scan codes)
 * xF6       Set default parameters
 * xF7       Set all keys to typematic/autorepeat only (scancode set 3 only)
 * xF8       Set all keys to make/release (scancode set 3 only)
 * xF9       Set all keys to make only (scancode set 3 only)
 * xFA       Set all keys to typematic/autorepeat/make/release (scancode set 3 only)
 * xFB scode Set specific key to typematic/autorepeat only (scancode set 3 only)
 * xFC scode Set specific key to make/release (scancode set 3 only)
 * xFD scode Set specific key to make only (scancode set 3 only)
 * xFE       Resend last byte
 * xFF       Reset and start self-test
 */
#ifndef PS2_KBD_H
#define PS2_KBD_H

#include <stdint.h>
#include <event.h>

/* keyboard events */
#define EVT_KBD_SCAN 	0x80 /** scan code event */
#define EVT_KBD_ERROR 	0x81 /** error event */
	#define EVT_KBD_EPARITY 0x00
	#define EVT_KBD_EACK	0x01
	#define EVT_KBD_ESTART	0x02
	#define EVT_KBD_ESTOP	0x03
#define EVT_KBD_CMD_ACK	0x82 /** command ack event */

#define KBD_CLOCK_PIN 4
#define KBD_CLOCK P04 /** ps2 clock pin */
#define KBD_DATA  P03 /** ps2 data pin */

/* most common command */
#define KBD_CMD_LED 	0xED /* expects argument - LED lock mask */
	#define KBD_LED_SCROLL  0x01
	#define KBD_LED_NUM		0x02
	#define KBD_LED_CAPS  	0x04
#define KBD_CMD_ECHO 	0xEE
#define KBD_CMD_ID 		0xF2
#define KBD_CMD_ENABLE  0xF4
#define KBD_CMD_DISABLE 0xF5
#define KBD_CMD_RESET 	0xFF

#define KBD_CMD_ACK 	0xFA
#define KBD_CMD_RESEND 	0xFE

/**
 * send command to the keyboard
 * @param cmd command or data to send
 */
void kbd_send_cmd(uint8_t cmd);

/** returns non zero if command is pending */
uint8_t kbd_cmd_pending(void);

/** keyboard event handler */
void kbd_event(uint8_t type, uint8_t data);

#endif