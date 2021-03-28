#ifndef N76E003_IRQ_H
#define N76E003_IRQ_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum IRQ_NUM {
	IRQ_EXT0  = 0,	/** External interrupt 0 */
	IRQ_TIM0  = 1,	/** Timer 0 overflow */
	IRQ_EXT1  = 2,	/** External interrupt 1 */
	IRQ_TIM1  = 3,	/** Timer 1 overflow */
	IRQ_UART0 = 4,	/** Serial port 0 interrupt */
	IRQ_TIM2  = 5,	/** Timer 2 overflow */
	IRQ_I2C   = 6,	/** I2C status/timer-out interrupt */
	IRQ_PIN   = 7,	/** Pin interrupt */
	IRQ_BOD   = 8,	/** Brown-out detection interrupt */
	IRQ_SPI   = 9,	/** SPI interrupt */
	IRQ_WDT   = 10,	/** WDT interrupt */
	IRQ_ADC   = 11,	/** ADC interrupt */
	IRQ_ICAP  = 12,	/** Input capture interrupt */
	IRQ_PWM   = 13,	/** PWM interrupt */
	IRQ_FB    = 14,	/** Fault Brake interrupt */
	IRQ_UART1 = 15,	/** Serial port 1 interrupt */
	IRQ_TIM3  = 16,	/** Timer 3 overflow */
	IRQ_WKT	  = 17	/** Self Wake-up Timer interrupt */
};

/* all interrupts generating events will use the same priority and the same bank */
#define IRQ_REG_BANK		1
#define IRQ_TIM0_REG_BANK	IRQ_REG_BANK
#define IRQ_UART_REG_BANK	IRQ_REG_BANK
#define IRQ_I2C_REG_BANK	IRQ_REG_BANK
#define IRQ_PIN_REG_BANK	IRQ_REG_BANK
#define IRQ_WKT_REG_BANK	IRQ_REG_BANK
#define IRQ_PWM_REG_BANK 	IRQ_REG_BANK

/*--------------------------------------------------------------------------
  Some defines to make VS-Code IntelliSense happy
--------------------------------------------------------------------------*/
#ifndef SDCC
#define INTERRUPT(A,B)
#else
#define INTERRUPT(INUM,BANK) __interrupt INUM __using BANK
#endif

/*
	simple locking for atomic access to critical variables
	volatile __bit bitlock; // bit to be used for a lock
	unlock(bitlock); // init unlock state, bitlock = 1;

	lock(bitlock);
	... access to critical variables
	unlock(bitlock);
*/

/**
  * @brief initialization of a bitlock
  * @param bitlock volatile __bit bitlock to set to 1
  */
#define initlock(bitlock) bitlock = 1

/**
  * @brief locking for access to critical variables
  * @param bitlock volatile __bit bitlock to be used for a lock
  */
#define lock(bitlock) do { if (bitlock) { bitlock = 0; break; } } while(1)

/**
  * @brief unlocking of critical variables
  * @param bitlock volatile __bit bitlock to unlock
  */
#define unlock(bitlock) bitlock = 1

/**
  interrupts disable/enables
*/
#define eni() EA=1 /** enable all interrupts */
#define cli() do{EA_SAVE=EA;EA=0;}while(0) /** disable all interrupts */
#define sti() EA=EA_SAVE	  /** restore all interrupts flag */

#define cli_tim2()	EIE &= ~EIE_ET2	 /** Disable Timer 2 interrupt */
#define cli_spi()	EIE &= ~EIE_ESPI /** Disable SPI interrupt */
#define cli_fb()	EIE &= ~EIE_EFB	 /** Disable Fault Brake interrupt */
#define cli_wtd()	EIE &= ~EIE_EWDT /** Disable WDT interrupt */
#define cli_pwm()	EIE &= ~EIE_EPWM /** Disable PWM interrupt */
#define cli_cap()	EIE &= ~EIE_ECAP /** Disable input capture interrupt */
#define cli_pin()	EIE &= ~EIE_EPI	 /** Disable pin interrupt */
#define cli_i2c()	EIE &= ~EIE_EI2C /** Disable I2C interrupt */

#define sti_tim2()	EIE |= EIE_ET2	 /** Enable Timer 2 interrupt */
#define sti_spi()	EIE |= EIE_ESPI	 /** Enable SPI interrupt */
#define sti_fb()	EIE |= EIE_EFB	 /** Enable Fault Brake interrupt */
#define sti_wtd()	EIE |= EIE_EWDT	 /** Enable WDT interrupt */
#define sti_pwm()	EIE |= EIE_EPWM	 /** Enable PWM interrupt */
#define sti_cap()	EIE |= EIE_ECAP	 /** Enable input capture interrupt */
#define sti_pin()	EIE |= EIE_EPI	 /** Enable pin interrupt */
#define sti_i2c()	EIE |= EIE_EI2C	 /** Enable I2C interrupt */

//#define sw_reset() EA=0;TA=0xAA;TA=0x55;CHPCON|=SET_BIT7
#define sw_reset() __asm__("clr	_EA\n mov _TA,#0xAA\n mov _TA,#0x55\n orl _CHPCON,#0x80")

#ifdef __cplusplus
}
#endif

#endif
