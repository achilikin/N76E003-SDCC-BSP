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
#include <N76E003.h>
#include <tick.h>
#include <uart.h>
#include <pinterrupt.h>

#include <bv4618.h>
#include <pcf8574.h>

#include "cfg.h"
#include "ps2k.h"

static __bit kbd_cmd;	  /** command mode flag */
static __bit kbd_cmd_ack; /** command ack flag */
static uint8_t kbd_data;  /** data to send/receive */
static uint8_t kbd_clock; /** clock counter */
static uint8_t parity;	  /** parity counter */

void pin_interrupt_handler(void) INTERRUPT(IRQ_PIN, IRQ_PIN_REG_BANK)
{
	/* send command mode */
	if (kbd_cmd) {
		kbd_clock++;
		if (kbd_clock == 1) { /* start bit */
			KBD_DATA = 0;
			goto exit;
		}
		if (kbd_clock < 10) { /* sending data out */
			parity += kbd_data & 0x01;
			KBD_DATA = kbd_data & 0x01;
			kbd_data >>= 1;
			goto exit;
		}
		if (kbd_clock == 10) { /* send parity */
			KBD_DATA = parity & 0x01;
			goto exit;
		}
		if (kbd_clock == 11) { /* stop bit */
			KBD_DATA = 1;
			goto exit;
		}
		/* kbd_clock == 12: ack bit, KBD_DATA should be 0 */
		if (KBD_DATA) {
			event_put(EVT_KBD_ERROR, EVT_KBD_EACK); /* report command ack error */
			kbd_cmd_ack = 0; /* command was not accepted, no reason to wait for an ack */
		}
		kbd_cmd = 0;
		goto reset;
	}

	/* receive data mode */
	if (!kbd_clock) {
		if (!KBD_DATA) { /* start data bit should be low */
			kbd_clock++;
			parity = 1; /* init with 1 to calculate odd parity */
		} else {
			event_put(EVT_KBD_ERROR, EVT_KBD_ESTART); /* report start bit error */
			kbd_cmd_ack = 0; /* reset command ack */
		}
		goto exit;
	}
	if (kbd_clock < 9) { /* receive data */
		if (KBD_DATA) {
			kbd_data |= 1 << (kbd_clock - 1);
			parity++;
		}
		kbd_clock++;
		goto exit;
	}
	if (kbd_clock == 9) { /* check parity bit */
		if ((parity & 0x01) != KBD_DATA) {
			event_put(EVT_KBD_ERROR, EVT_KBD_EPARITY); /* report parity error */
			kbd_cmd_ack = 0;
			goto reset;
		}
		kbd_clock++;
		goto exit;
	}
	if (kbd_clock == 10) {
		if (KBD_DATA) { /* stop bit should be high for valid transaction */
			if (kbd_cmd_ack) /* also generate cmd ack event */
				event_put(EVT_KBD_CMD_ACK, kbd_data);
			event_put(EVT_KBD_SCAN, kbd_data);
		}
		else
			event_put(EVT_KBD_ERROR, EVT_KBD_ESTOP); /* report stop bit error */
		kbd_cmd_ack = 0; /* simplified command acknowledgement */
	}
reset:
	kbd_clock = kbd_data = 0;
exit:
	PIF = 0;
}

void kbd_send_cmd(uint8_t cmd)
{
	cli_pin();
	parity = 1;
	kbd_cmd = 1;
	kbd_cmd_ack = 1; /* wait for ack byte */
	kbd_clock = 0;
	kbd_data = cmd;

	/* prepare for start condition */
	KBD_DATA = 1;
	KBD_CLOCK = 1;
	delay_mks(100);

	KBD_CLOCK = 0;
	delay_mks(100);
	KBD_DATA = 0;

	/* set clock high so keyboard will take over and generate the clock */
	KBD_CLOCK = 1;
	sti_pin();
}

uint8_t kbd_cmd_pending(void)
{
	return kbd_cmd_ack;
}

/* the first 64 printable codes for UK keyboard */
static const uint8_t key_scan[64] = {
	'\0',  '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',  '\0',
	'\t',  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n','\0',
	'a',   's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`', '\0','#',
	'z',   'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*','\0',' ', '\0'
};

uint8_t knum, kidx;
uint8_t kbuf[8];

void kbd_event(uint8_t type, uint8_t data)
{
	if (type == EVT_KBD_SCAN) {
		kbuf[kidx] = data;
		kidx = (kidx + 1) & 7;
		if (knum < 8)
			knum++;

		uart_putsc("scan code ");
		uart_puth(data);
		uart_putsc(" (");
		uart_putn(data);
		uart_putsc(")");
		if (data < 64) {
			type = key_scan[data];
			if (type >= ' ') {
				uart_putc('\'');
				uart_putc(type);
				uart_putc('\'');
			}
			/* processes ome keys */
			if (type == 't') /* toggle timer state if 't' is pressed */
				cfg.flags ^= CFG_TIMER_ON;
			else if (type == 'u') /* toggle uart output for timer if 't' is pressed */
				cfg.flags ^= CFG_OUT_UART;
		}
		uart_putln();
#ifdef USE_BV4618_LCD
		bv4618_line(3);
		if (data == 0x81) { /* Esc release code in scan set 1 - clear line */
			bv4618_clline();
		} else {
			/* we can fit only 7 codes to 20 chars display */
			for (uint8_t i = 0; i < 7; i++) {
				if (knum == 8) {
					/* kidx points to the oldest code, skip it as we can display only 7 */
					bv4618_puth(kbuf[(kidx + i + 1) & 0x07]);
				} else {
					if (i == knum)
						break;
					bv4618_puth(kbuf[i]);
				}
				if (i < 6)
					bv4618_putc(' ');
			}
		}
#endif
#ifdef USE_PCF8574_LCD
		pcf_line(3);
		if (data == 0x81) { /* Esc release code in scan set 1 - clear line */
			pcf_clline();
		} else {
			/* we can fit only 5 codes to 20 chars display */
			for (uint8_t i = 0; i < 5; i++) {
				pcf_putc(' ');
				if (knum == 8) {
					/* kidx points to the oldest code, skip 3 as we can display only 5 */
					pcf_puth(kbuf[(kidx + i + 3) & 0x07]);
				} else {
					if (i == knum)
						break;
					if (knum > 5)
						pcf_puth(kbuf[i + knum - 5]);
					else
						pcf_puth(kbuf[i]);
				}
			}
		}
#endif
		if (data == 0x81) /* Esc release code in scan set 1 - clear buffer */
			knum = kidx = 0;
	} else if (type == EVT_KBD_ERROR) {
		uart_putsc("error ");
		uart_putnl(data);
	} else if (type == EVT_KBD_CMD_ACK) {
		uart_putsc("cmd ack ");
		uart_puthl(data);
	}
	return;
}