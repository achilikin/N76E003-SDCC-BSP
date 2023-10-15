#ifndef MAIN_H
#define MAIN_H

#include <N76E003.h>

#define DEBUG 0

#ifdef MARK_PIN
/** set pin to high and back to trace a time marker on oscilloscope */
#define MARK do{MARK_PIN=1;MARK_PIN=0;}while(0)
#define MARK_ON MARK_PIN=1
#define MARK_OFF MARK_PIN=0
#else
#define MARK
#define MARK_ON
#define MARK_OFF
#endif

int8_t commander(__idata char* cmd); /** cli handler */
void timer(void); /** timer handler called every second if enabled */

#define PWM_PERIOD 100
#define SLEEP_TIMER_PIN P30
#define LED_PWM_CHANNEL 0

#define set_state(STATE) do { state = STATE; P01 = SLEEP_TIMER_PIN; P13 = STATE & 0x01; P14 = STATE >> 1; } while(0)

/**
 *@brief read Vdd and normalize 3.00 to 4.20V range
 *
 * @return 300 for 3.0V, 420 for 4.2V
 */
uint16_t get_vdd(void);

/**
 *@brief Convert Vdd to PWM duty
 *
 * @param vdd - normalized Vdd 300 for 3.0V, 420 for 4.2V
 * @return PWM duty 100 to 70%
 */
uint8_t get_pwm_power(uint16_t vdd);

enum state_t {
	STATE_LED_OFF = 0x00, 		/** LED is OFF, sleeping */
	STATE_LED_ON = 0x01, 		/** LED timer is ON */
	STATE_BATTERY_LOW = 0x02	/** LED timer is ON, but battery low */
};

extern uint8_t state;

void state_print(uint8_t state);
void turn_led_on(uint8_t power);
void turn_led_off(void);

enum wkt_tick_mode_t {
	WKT_TICK_MSEC,		/** ~1 msecd, default at init */
	WKT_TICK_SECOND, 	/** ~1 second */
	WKT_TICK_MINUTE, 	/** ~1 minute */
	WKT_TICK_MODE_MAX
};

/**
 *@brief set WKT tick interval
 *
 * @param mode - WKT_TICK_MSEC, WKT_TICK_SECOND, TICK_MINUTE
 */
void wkt_tick_set_mode(enum wkt_tick_mode_t mode);

/**
 *@brief EXT0 IRQ to wake up on SLEEP_TIMER expiration
 *
 */
void ext0_interrupt_handler(void) INTERRUPT(IRQ_EXT0, IRQ_EXT0_REG_BANK);

#endif
