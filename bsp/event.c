#include <N76E003.h>

#include "irq.h"
#include "event.h"

#define EVENT_NUM	64 /** must be power of 2 */
#define EVENT_BUF_SIZE (EVENT_NUM * 2) /** each event takes 2 bytes */
#define EVENT_BUF_MASK (EVENT_BUF_SIZE - 1)

static uint8_t evt_put; /** position to put even in */
static uint8_t evt_get; /** position to get even from */
static volatile uint8_t evt_num; /** number of events in the buffer */
static volatile uint8_t evt_err; /** overflow error */

static __xdata uint8_t evt_buf[EVENT_NUM * 2];

#pragma save
#pragma nooverlay

void event_put(uint8_t type, uint8_t data) __reentrant __using(IRQ_REG_BANK)
{
	if (evt_num == EVENT_NUM) {
		if (evt_err == EVT_NONE)
			evt_err = type;
	} else {
		/* use a pointer to generate smaller code */
		__xdata uint8_t *buf = evt_buf + evt_put;
		buf[0] = type;
		buf[1] = data;
		evt_put = (evt_put + 2) & EVENT_BUF_MASK;
		evt_num = evt_num + 1;
	}
}

#pragma restore

uint16_t event_get(void)
{
	event_t event;

	if (evt_num == 0)
		event.type = EVT_NONE;
	else if (evt_err) {
		event.type = EVT_ERROR;
		event.data = evt_err;
		evt_err = 0;
	} else {
		__xdata uint8_t *buf = evt_buf + evt_get;
		event.type = buf[0];
		event.data = buf[1];
		cli();
		evt_num -= 1;
		sti();
		evt_get = (evt_get + 2) & EVENT_BUF_MASK;
	}

	return event.evt;
}

void event_flush(void)
{
	cli();
	evt_put = evt_get =	evt_num = evt_err = 0;
	sti();
}
