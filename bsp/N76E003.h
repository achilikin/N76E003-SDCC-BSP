/*------------------------------------------------------------------------------
  N76E003.H

  Header file for Nuvoton N76E003 BSP adopted for SDCC compiler
  http://sdcc.sourceforge.net/

  Original header files for SFR bytes and bits,
  SFR Macros and Functions Defines are from OpenNuvoton project
  https://github.com/OpenNuvoton/N76E003-BSP
------------------------------------------------------------------------------*/
#ifndef N76E003_H
#define N76E003_H

#include <stdint.h>

/*------------------------------------------------------------------------------
  Some defines to make VS-Code IntelliSense happy
------------------------------------------------------------------------------*/
#ifndef SDCC
#define __bit char
#define __sfr
#define __sbit
#define __code
#define __data
#define __idata
#define __xdata
#define __at(x) extern char
#define __naked
#define __nonbanked
#define __reentrant
#define __using(x)
#define AT_ADDRESS(A)
#else
#define AT_ADDRESS(A) __code __at(A)
#endif

extern __bit EA_SAVE;

/** a simple SDCC trick to get the current code size  */
uint16_t sdcc_get_code_size(void);

#if defined FOSC_16600
/**
 * Set HIRC to 16.6MHz, see "13.5 Baud Rate", datasheet v1.02
 * 8 bits of RCTRIM0 + 1 bit of RCTRIM1 represent 9 bit internal RC trim value.
 * Each bit deviation is 0.25% of 16MHz or about 40KHz/bit.
 */
#ifndef HIRC_TRIM
#define HIRC_TRIM 0
#endif
extern uint8_t trim, trim0, trim1;
void set_fosc_16600(void);
#endif

/**
 * Simple union of uint16_t and two bytes
 */
typedef union {
	uint16_t u16;
	struct {
		uint8_t u8low;
		uint8_t u8high;
	};
} val16_t;

#define nop() __asm__("nop")
#define swap8(data) ((data << 4) | (data >> 4))
#define swap16(data) ((data << 8) | (data >> 8))
#define ta_enable() TA=0xAA;TA=0x55
#define sfr_page(page) do{EA_SAVE=EA;EA=0;TA=0xAA;TA=0x55;SFRS=page;EA=EA_SAVE;}while(0)

#define HIRC_FREQ 16000000L
#ifdef  FOSC_16600
#undef  HIRC_FREQ
#define HIRC_FREQ 16600000L
#endif

#define APROM_SIZE 18*1024 /* 0x4800 */
#define PAGE_SIZE  128

/* SFR bytes with corresponding bits */
__sfr __at(0x80) P0; /** Port 0 */
	__sbit __at(0x80+7) P07;
	__sbit __at(0x80+7) RXD;
	__sbit __at(0x80+6) P06;
	__sbit __at(0x80+6) TXD;
	__sbit __at(0x80+5) P05;
	__sbit __at(0x80+4) P04;
	__sbit __at(0x80+4) STADC;
	__sbit __at(0x80+3) P03;
	__sbit __at(0x80+2) P02;
	__sbit __at(0x80+2) RXD_1;
	__sbit __at(0x80+1) P01;
	__sbit __at(0x80+1) MISO;
	__sbit __at(0x80+0) P00;
	__sbit __at(0x80+0) MOSI;

__sfr __at(0x81) SP;			/** Stack Pointer */
__sfr __at(0x82) DPL;			/** Data Pointer, Low byte */
__sfr __at(0x83) DPH;           /** Data Pointer, High byte */
__sfr __at(0x84) RCTRIM0;		/** High Speed Internal Oscillator 16 MHz Trim 0 */
__sfr __at(0x85) RCTRIM1;		/** High Speed Internal Oscillator 16 MHz Trim 1 */
__sfr __at(0x86) RWK; 			/** Wake-up Timer Reload byte */

/** Power CONtrol */
__sfr __at(0x87) PCON;
	#define PCON_SMOD	0x80	/** Serial port 0 double baud rate enable */
	#define PCON_SMOD0	0x40	/** Serial port 0 framing error flag access enable */
	#define PCON_POF	0x10	/** Power-on reset flag */
	#define PCON_GF1	0x08	/** General purpose flag 1 */
	#define PCON_GF0	0x04	/** General purpose flag 0 */
	#define PCON_PD		0x02	/** Power-down mode */
	#define PCON_IDL	0x01	/** Idle mode */

__sfr __at(0x88) TCON;			/** Timer CONtrol */
	__sbit __at(0x88+7) TF1;
	__sbit __at(0x88+6) TR1;
	__sbit __at(0x88+5) TF0;
	__sbit __at(0x88+4) TR0;
	__sbit __at(0x88+3) IE1;
	__sbit __at(0x88+2) IT1;
	__sbit __at(0x88+1) IE0;
	__sbit __at(0x88+0) IT0;

__sfr __at(0x89) TMOD;		/** timer mode */
__sfr __at(0x8A) TL0;		/** timer 0, low byte */
__sfr __at(0x8B) TL1;       /** timer 1, low byte */
__sfr __at(0x8C) TH0;       /** timer 0, high byte */
__sfr __at(0x8D) TH1;       /** timer 1, high byte */

__sfr __at(0x8E) CKCON;		/** clock control */
	/* set_* & clr_* defines obscure bit names, so better to use full name */
	#define CKCON_CLOEN  SET_BIT1 /** enable Fsys CLOCK output on P1.1 */
	#define CKCON_T1M 	 SET_BIT4 /** Timer 1 is system clock, else Fsys / 12 */
	#define CKCON_T0M 	 SET_BIT3 /** Timer 0 is system clock, else Fsys / 12 */
	#define CKCON_PWMCKS SET_BIT6 /** PWM clock source is T1, else Fsys */
	#define set_PWMCKS  CKCON |= SET_BIT6
	#define set_T1M     CKCON |= SET_BIT4
	#define set_T0M     CKCON |= SET_BIT3
	#define set_CLOEN   CKCON |= SET_BIT1

	#define clr_PWMCKS  CKCON &= ~SET_BIT6
	#define clr_T1M     CKCON &= ~SET_BIT4
	#define clr_T0M     CKCON &= ~SET_BIT3
	#define clr_CLOEN   CKCON &= ~SET_BIT1

__sfr __at(0x8F) WKCON;
	#define WKCON_WKTCK SET_BIT5 /** marked as reserved, but exists on Figure 11.2-1, datasheet v1.09 */
	#define WKCON_WKTF  SET_BIT4
	#define WKCON_WKTR  SET_BIT3
	#define WKCON_WKPS  (SET_BIT2 | SET_BIT1 | SET_BIT0)

__sfr __at(0x90) P1;
	__sbit __at(0x90+7) P17;
	__sbit __at(0x90+6) P16;
	__sbit __at(0x90+6) TXD_1;
	__sbit __at(0x90+5) P15;
	__sbit __at(0x90+4) P14;
	__sbit __at(0x90+4) SDA;
	__sbit __at(0x90+3) P13;
	__sbit __at(0x90+3) SCL;
	__sbit __at(0x90+2) P12;
	__sbit __at(0x90+1) P11;
	__sbit __at(0x90+0) P10;

__sfr __at(0x91) SFRS;  	/* TA Protected */
__sfr __at(0x92) CAPCON0;
__sfr __at(0x93) CAPCON1;
__sfr __at(0x94) CAPCON2;
__sfr __at(0x95) CKDIV;
__sfr __at(0x96) CKSWT; 	/* TA Protected */
__sfr __at(0x97) CKEN;  	/* TA Protected */

__sfr __at(0x98) SCON;		/** Serial port 0 control */
	__sbit __at(0x98+7) SM0;
	__sbit __at(0x98+7) FE;
	__sbit __at(0x98+6) SM1;
	__sbit __at(0x98+5) SM2;
	__sbit __at(0x98+4) REN; /** Serial port 0 reception Enabled */
	__sbit __at(0x98+3) TB8;
	__sbit __at(0x98+2) RB8;
	__sbit __at(0x98+1) TI;	/** Serial port 0 TX interrupt flag */
	__sbit __at(0x98+0) RI; /** Serial port 0 RX interrupt flag */

__sfr __at(0x99) SBUF;		/** Serial Port 0 Data Buffer */
__sfr __at(0x9A) SBUF_1;	/** Serial Port 1 Data Buffer */

__sfr __at(0x9B) EIE;		/** Extensive Interrupt Enable */
	#define EIE_ET2		0x80	/** Enable Timer 2 interrupt */
	#define EIE_ESPI	0x40	/** Enable SPI interrupt */
	#define EIE_EFB		0x20	/** Enable Fault Brake interrupt */
	#define EIE_EWDT	0x10	/** Enable WDT interrupt */
	#define EIE_EPWM	0x08	/** Enable PWM interrupt */
	#define EIE_ECAP	0x04	/** Enable input capture interrupt */
	#define EIE_EPI		0x02	/** Enable pin interrupt */
	#define EIE_EI2C	0x01	/** Enable I2C interrupt */

__sfr __at(0x9C) EIE1;		/** Extensive Interrupt Enable 1 */
	#define EIE1_EWKT	0x04	/** Enable WKT interrupt */
	#define EIE1_ET3	0x02	/** Enable Timer 3 interrupt */
	#define EIE1_ES_1	0x01	/** Enable serial port 1 interrupt */

	#define set_EWKT    EIE1 |= SET_BIT2
	#define set_ET3     EIE1 |= SET_BIT1
	#define set_ES_1    EIE1 |= SET_BIT0

	#define clr_EWKT    EIE1 &= ~SET_BIT2
	#define clr_ET3     EIE1 &= ~SET_BIT1
	#define clr_ES_1    EIE1 &= ~SET_BIT0 /** Enable serial port 0 interrupt */

__sfr __at(0x9F) CHPCON; 	/* TA Protected */

__sfr __at(0xA0) P2;
	__sbit __at(0xA0+0) P20;

__sfr __at(0xA2) AUXR1;
	#define AUXR_SWRF		0x80 /** SW reset flag */
	#define AUXR_RSTPINF	0x40 /** External reset flag */
	#define AUXR_HARDF		0x20 /** Hard Fault reset flag */
	#define AUXR_GF2		0x80 /** General purpose flag 2 */
	#define AUXR_UART0PX	0x40 /** Serial port 0 pin exchange */
	#define AUXR_DPS		0x01 /** Data point select */

__sfr __at(0xA3) BODCON0; 	/* TA Protected */
__sfr __at(0xA4) IAPTRG;  	/* TA Protected */
__sfr __at(0xA5) IAPUEN;  	/* TA Protected */
__sfr __at(0xA6) IAPAL;
__sfr __at(0xA7) IAPAH;

__sfr __at(0xA8) IE;		/** Interrupt Enable (Bit-addressable) */
	__sbit __at(0xA8+7) EA;		/** Enable all interrupts */
	__sbit __at(0xA8+6) EADC;	/** Enable ADC interrupt */
	__sbit __at(0xA8+5) EBOD;	/** Enable brown-out interrupt */
	__sbit __at(0xA8+4) ES;		/** Enable serial port 0 interrupt */
	__sbit __at(0xA8+3) ET1;	/** Enable Timer 1 interrupt */
	__sbit __at(0xA8+2) EX1;	/** Enable external interrupt 1 */
	__sbit __at(0xA8+1) ET0;	/** Enable Timer 0 interrupt */
	__sbit __at(0xA8+0) EX0;	/** Enable external interrupt 0 */

__sfr __at(0xA9) SADDR;
__sfr __at(0xAA) WDCON;   	/* TA Protected */
__sfr __at(0xAB) BODCON1; 	/* TA Protected */
__sfr __at(0xAC) P3M1;
__sfr __at(0xAC) P3S; 		/* Access via page 1 */
__sfr __at(0xAD) P3M2;
__sfr __at(0xAD) P3SR; 		/* Access via page 1 */
__sfr __at(0xAE) IAPFD;
__sfr __at(0xAF) IAPCN;

__sfr __at(0xB0) P3;
	__sbit __at(0xB0+0) P30;

__sfr __at(0xB1) P0M1;
__sfr __at(0xB1) P0S; 		/* Access via page 1 */
__sfr __at(0xB2) P0M2;
__sfr __at(0xB2) P0SR; 		/* Access via page 1 */
__sfr __at(0xB3) P1M1;
__sfr __at(0xB3) P1S; 		/* Access via page 1 */
__sfr __at(0xB4) P1M2;
__sfr __at(0xB4) P1SR;		/* Access via page 1 */
__sfr __at(0xB5) P2S;
__sfr __at(0xB7) IPH;
__sfr __at(0xB7) PWMINTC;	/* Access via page 1 */

__sfr __at(0xB8) IP;
	__sbit __at(0xB8+6) PADC;
	__sbit __at(0xB8+5) PBOD;
	__sbit __at(0xB8+4) PS;
	__sbit __at(0xB8+3) PT1;
	__sbit __at(0xB8+2) PX1;
	__sbit __at(0xB8+1) PT0;
	__sbit __at(0xB8+0) PX0;

__sfr __at(0xB9) SADEN;
__sfr __at(0xBA) SADEN_1;
__sfr __at(0xBB) SADDR_1;
__sfr __at(0xBC) I2DAT;
__sfr __at(0xBD) I2STAT;
__sfr __at(0xBE) I2CLK;
__sfr __at(0xBF) I2TOC;

__sfr __at(0xC0) I2CON;
	__sbit __at(0xC0+6) I2CEN;
	__sbit __at(0xC0+5) STA;
	__sbit __at(0xC0+4) STO;
	__sbit __at(0xC0+3) SI;
	__sbit __at(0xC0+2) AA;
	__sbit __at(0xC0+0) I2CPX;

__sfr __at(0xC1) I2ADDR;
__sfr __at(0xC2) ADCRL;
__sfr __at(0xC3) ADCRH;
__sfr __at(0xC4) T3CON;		/** Timer 3 Control */
__sfr __at(0xC4) PWM4H; 	/** PWM4H at page 1 */
__sfr __at(0xC5) RL3;		/** Timer 3 Reload Low Byte */
__sfr __at(0xC5) PWM5H;		/** PWM5H at page 1 */
__sfr __at(0xC6) RH3;		/** Timer 3 Reload High Byte */
__sfr __at(0xC6) PIOCON1;	/** PIOCON1 at page 1 */
__sfr __at(0xC7) TA;

__sfr __at(0xC8) T2CON;
	__sbit __at(0xC8+7) TF2;
	__sbit __at(0xC8+2) TR2;
	__sbit __at(0xC8+0) CM_RL2;

__sfr __at(0xC9) T2MOD;
__sfr __at(0xCA) RCMP2L;
__sfr __at(0xCB) RCMP2H;
__sfr __at(0xCC) TL2;
__sfr __at(0xCC) PWM4L; 	/* Access via page 1 */
__sfr __at(0xCD) TH2;
__sfr __at(0xCD) PWM5L; 	/* Access via page 1 */
__sfr __at(0xCE) ADCMPL;
__sfr __at(0xCF) ADCMPH;

__sfr __at(0xD0) PSW;
	__sbit __at(0xD0+7) CY;
	__sbit __at(0xD0+6) AC;
	__sbit __at(0xD0+5) F0; /** F0 or UD0 (User Defined), MCS-51 standard */
	__sbit __at(0xD0+5) UD0; /** F0 or UD0 (User Defined), MCS-51 standard */
	__sbit __at(0xD0+4) RS1;
	__sbit __at(0xD0+3) RS0;
	__sbit __at(0xD0+2) OV;
	__sbit __at(0xD0+1) F1; /** F1 or UD1 (User Defined), MCS-51 standard */
	__sbit __at(0xD0+1) UD1; /** F1 or UD1 (User Defined), MCS-51 standard */
	__sbit __at(0xD0+0) P;

__sfr __at(0xD1) PWMPH;
__sfr __at(0xD2) PWM0H;
__sfr __at(0xD3) PWM1H;
__sfr __at(0xD4) PWM2H;
__sfr __at(0xD5) PWM3H;
__sfr __at(0xD6) PNP;
__sfr __at(0xD7) FBD;

__sfr __at(0xD8) PWMCON0;
	__sbit __at(0xD8+7) PWMRUN;
	__sbit __at(0xD8+6) LOAD;
	__sbit __at(0xD8+5) PWMF;
	__sbit __at(0xD8+4) CLRPWM;

__sfr __at(0xD9) PWMPL;
__sfr __at(0xDA) PWM0L;
__sfr __at(0xDB) PWM1L;
__sfr __at(0xDC) PWM2L;
__sfr __at(0xDD) PWM3L;
__sfr __at(0xDE) PIOCON0;
	#define PIOCON0_PIO05 SET_BIT5
	#define PIOCON0_PIO04 SET_BIT4
	#define PIOCON0_PIO03 SET_BIT3
	#define PIOCON0_PIO02 SET_BIT2
	#define PIOCON0_PIO01 SET_BIT1
	#define PIOCON0_PIO00 SET_BIT0
__sfr __at(0xDF) PWMCON1;
	#define PWMCON1_PWMMOD (SET_BIT7 | SET_BIT6)
	#define PWMCON1_GP 		SET_BIT5
	#define PWMCON1_PWMTYP 	SET_BIT4
	#define PWMCON1_FBINEN 	SET_BIT3
	#define PWMCON1_PWMDIV (SET_BIT2 | SET_BIT1 | SET_BIT0)

__sfr __at(0xE0) ACC;
__sfr __at(0xE1) ADCCON1;
__sfr __at(0xE2) ADCCON2;
__sfr __at(0xE3) ADCDLY;
__sfr __at(0xE4) C0L;
__sfr __at(0xE5) C0H;
__sfr __at(0xE6) C1L;
__sfr __at(0xE7) C1H;

__sfr __at(0xE8) ADCCON0;
	__sbit __at(0xE8+7) ADCF;
	__sbit __at(0xE8+6) ADCS;
	__sbit __at(0xE8+5) ETGSEL1;
	__sbit __at(0xE8+4) ETGSEL0;
	__sbit __at(0xE8+3) ADCHS3;
	__sbit __at(0xE8+2) ADCHS2;
	__sbit __at(0xE8+1) ADCHS1;
	__sbit __at(0xE8+0) ADCHS0;

__sfr __at(0xE9) PICON; /** Pin Interrupt Control */
	#define PICON_PIT67	0x80 /** Pin interrupt channel 6 and 7 type select */
	#define PICON_PIT45	0x40 /** Pin interrupt channel 4 and 5 type select */
	#define PICON_PIT3	0x20 /** Pin interrupt channel 3 type select */
	#define PICON_PIT2	0x10 /** Pin interrupt channel 2 type select */
	#define PICON_PIT1	0x08 /** Pin interrupt channel 1 type select */
	#define PICON_PIT0	0x04 /** Pin interrupt channel 0 type select */
	#define PICON_PIPS1	0x02 /** Pin interrupt port select */
	#define PICON_PIPS0	0x01 /** 00: 0, 01: 1, 10: 2, 11: 3 */

__sfr __at(0xEA) PINEN;	/** Pin Interrupt Negative Polarity Enable, bit per channel */
__sfr __at(0xEB) PIPEN;	/** Pin Interrupt Positive Polarity Enable, bit per channel */
__sfr __at(0xEC) PIF;	/** Pin Interrupt Flags, bit per channel */
__sfr __at(0xED) C2L;
__sfr __at(0xEE) C2H;
__sfr __at(0xEF) EIP;

__sfr __at(0xF0) B;
__sfr __at(0xF1) CAPCON3;
__sfr __at(0xF2) CAPCON4;
__sfr __at(0xF3) SPCR;
	#define SPCR_SSOE  SET_BIT7 /** SS output enable, 0: SS as a general I/O */
	#define SPCR_SPIEN SET_BIT6 /** SPI enable */
	#define SPCR_LSBFE SET_BIT5 /** LSB first enable, 0: MSB */
	#define SPCR_MSTR  SET_BIT4 /** SPI mode select, 1: master */
	#define SPCR_CPOL  SET_BIT3 /** SPI clock polarity select. 0: low in idle state*/
	#define SPCR_CPHA  SET_BIT2 /** SPI clock phase select. 0: data sample on the first edge of the clock */
	#define SPCR_SRR   (SET_BIT1 | SET_BIT0) /** SPI clock rate select */
	#define SPI_CLOCK_8MHZ 0x00
	#define SPI_CLOCK_4MHZ 0x01
	#define SPI_CLOCK_2MHZ 0x02
	#define SPI_CLOCK_1MHZ 0x03
__sfr __at(0xF3) SPCR2; 	/* Access via page 1 */
__sfr __at(0xF4) SPSR;
	#define SPSR_SPIF 	 SET_BIT7 /** SPI complete flag */
	#define SPSR_WCOL 	 SET_BIT6 /** write collision error flag */
	#define SPSR_SPIOVF  SET_BIT5 /** SPI overrun error flag */
	#define SPSR_MODF 	 SET_BIT4 /** mode fault error flag */
	#define SPSR_DISMODF SET_BIT3 /** disable mode fault error flag */
	#define SPSR_TXBUF 	 SET_BIT2 /** SPI write buffer status, 0: empty, 1: full */
__sfr __at(0xF5) SPDR;
__sfr __at(0xF6) AINDIDS;
__sfr __at(0xF7) EIPH;

__sfr __at(0xF8) SCON_1;
	__sbit __at(0xF8+7) SM0_1;
	__sbit __at(0xF8+7) FE_1;
	__sbit __at(0xF8+6) SM1_1;
	__sbit __at(0xF8+5) SM2_1;
	__sbit __at(0xF8+4) REN_1;	/** Serial port 1 Receiving enable */
	__sbit __at(0xF8+3) TB8_1;
	__sbit __at(0xF8+2) RB8_1;
	__sbit __at(0xF8+1) TI_1;	/** Serial port 1 Transmission interrupt flag */
	__sbit __at(0xF8+0) RI_1;	/** Serial port 1 Receiving interrupt flag */

__sfr __at(0xF9) PDTEN; 	/* TA Protected */
__sfr __at(0xFA) PDTCNT; 	/* TA Protected */
__sfr __at(0xFB) PMEN;
__sfr __at(0xFC) PMD;
__sfr __at(0xFC) PORDIS;    /* TA Protected POR Disable */
__sfr __at(0xFE) EIP1;
__sfr __at(0xFF) EIPH1;

/*------------------------------------------------------------------------------
  SFR related macros from SRF_Macro.h
------------------------------------------------------------------------------*/

#define SET_TA TA=0xAA;TA=0x55
#define SET_EA_TA EA_SAVE=EA;EA=0;TA=0xAA;TA=0x55

/**** P0		80H *****/
#define set_P00	P00	= 1
#define set_P01	P01	= 1
#define set_P02	P02	= 1
#define set_P03	P03	= 1
#define set_P04	P04	= 1
#define set_P05	P05	= 1
#define set_P06	P06	= 1
#define set_P07	P07	= 1

#define clr_P00	P00	= 0
#define clr_P01	P01	= 0
#define clr_P02	P02	= 0
#define clr_P03	P03	= 0
#define clr_P04	P04	= 0
#define clr_P05	P05	= 0
#define clr_P06	P06	= 0
#define clr_P07	P07	= 0

//**** SP  		81H ****
//**** DPH		82H ****
//**** DPL		83H ****
//**** RWK		86H ****

//**** PCON		87H *****
#define set_SMOD    PCON |= SET_BIT7
#define set_SMOD0   PCON |= SET_BIT6
#define set_POF     PCON |= SET_BIT4
#define set_GF1     PCON |= SET_BIT3
#define set_GF0     PCON |= SET_BIT2
#define set_PD      PCON |= SET_BIT1
#define set_IDL    	PCON |= SET_BIT0

#define clr_SMOD    PCON &= ~SET_BIT7
#define clr_SMOD0   PCON &= ~SET_BIT6
#define clr_POF     PCON &= ~SET_BIT4
#define clr_GF1     PCON &= ~SET_BIT3
#define clr_GF0     PCON &= ~SET_BIT2
#define clr_PD      PCON &= ~SET_BIT1
#define clr_IDL    	PCON &= ~SET_BIT0

/**** TCON		88H ****/
#define set_TF1		TF1	= 1
#define set_TR1		TR1	= 1
#define set_TF0		TF0	= 1
#define set_TR0		TR0	= 1
#define set_IE1		IE1	= 1
#define set_IT1		IT1	= 1
#define set_IE0		IE0	= 1
#define set_IT0		IT0	= 1

#define clr_TF1		TF1	= 0
#define clr_TR1		TR1	= 0
#define clr_TF0		TF0	= 0
#define clr_TR0		TR0	= 0
#define clr_IE1		IE1	= 0
#define clr_IT1		IT1	= 0
#define clr_IE0		IE0	= 0
#define clr_IT0		IT0	= 0

//**** TMOD		89H ****
#define set_GATE_T1 TMOD |= SET_BIT7
#define set_CT_T1 	TMOD |= SET_BIT6
#define set_M1_T1 	TMOD |= SET_BIT5
#define set_M0_T1 	TMOD |= SET_BIT4
#define set_GATE_T0 TMOD |= SET_BIT3
#define set_CT_T0 	TMOD |= SET_BIT2
#define set_M1_T0 	TMOD |= SET_BIT1
#define set_M0_T0 	TMOD |= SET_BIT0

#define clr_GATE_T1 TMOD &= ~SET_BIT7
#define clr_CT_T1 	TMOD &= ~SET_BIT6
#define clr_M1_T1 	TMOD &= ~SET_BIT5
#define clr_M0_T1 	TMOD &= ~SET_BIT4
#define clr_GATE_T0 TMOD &= ~SET_BIT3
#define clr_CT_T0 	TMOD &= ~SET_BIT2
#define clr_M1_T0 	TMOD &= ~SET_BIT1
#define clr_M0_T0 	TMOD &= ~SET_BIT0

//**** TH1		8AH ****
//**** TH0		8BH ****
//**** TL1		8CH	****
//**** TL0		8DH ****

//**** P1		90H *****
#define set_P10		P10	= 1
#define set_P11		P11	= 1
#define set_P12		P12	= 1
#define set_P13		P13	= 1
#define set_P14		P14	= 1
#define set_P15		P15	= 1
#define set_P16		P16	= 1
#define set_P17		P17	= 1

#define clr_P10		P10	= 0
#define clr_P11		P11	= 0
#define clr_P12		P12	= 0
#define clr_P13		P13	= 0
#define clr_P14		P14	= 0
#define clr_P15		P15	= 0
#define clr_P16		P16	= 0
#define clr_P17		P17	= 0

//****SFRS		91H ****
// same as 'sfr_page(page)'
#define set_SFRPAGE  do{SET_EA_TA;SFRS=0x01;EA=EA_SAVE;}while(0)
#define clr_SFRPAGE  do{SET_EA_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//****CAPCON0	92H ****
#define set_CAPEN2  CAPCON0 |= SET_BIT6
#define set_CAPEN1  CAPCON0 |= SET_BIT5
#define set_CAPEN0  CAPCON0 |= SET_BIT4
#define set_CAPF2   CAPCON0 |= SET_BIT2
#define set_CAPF1   CAPCON0 |= SET_BIT1
#define set_CAPF0   CAPCON0 |= SET_BIT0

#define clr_CAPEN2  CAPCON0 &= ~SET_BIT6
#define clr_CAPEN1  CAPCON0 &= ~SET_BIT5
#define clr_CAPEN0  CAPCON0 &= ~SET_BIT4
#define clr_CAPF2   CAPCON0 &= ~SET_BIT2
#define clr_CAPF1   CAPCON0 &= ~SET_BIT1
#define clr_CAPF0   CAPCON0 &= ~SET_BIT0

//**** CAPCON1	93H ****
#define set_CAP2LS1 CAPCON1 |= SET_BIT5
#define set_CAP2LS0 CAPCON1 |= SET_BIT4
#define set_CAP1LS1 CAPCON1 |= SET_BIT3
#define set_CAP1LS0 CAPCON1 |= SET_BIT2
#define set_CAP0LS1 CAPCON1 |= SET_BIT1
#define set_CAP0LS0 CAPCON1 |= SET_BIT0

#define clr_CAP2LS1 CAPCON1 &= ~SET_BIT5
#define clr_CAP2LS0 CAPCON1 &= ~SET_BIT4
#define clr_CAP1LS1 CAPCON1 &= ~SET_BIT3
#define clr_CAP1LS0 CAPCON1 &= ~SET_BIT2
#define clr_CAP0LS1 CAPCON1 &= ~SET_BIT1
#define clr_CAP0LS0 CAPCON1 &= ~SET_BIT0

//**** CAPCON2	94H ****
#define set_ENF2  CAPCON2   |= SET_BIT6
#define set_ENF1  CAPCON2   |= SET_BIT5
#define set_ENF0  CAPCON2   |= SET_BIT4

#define clr_ENF2  CAPCON2   &= ~SET_BIT6
#define clr_ENF1  CAPCON2   &= ~SET_BIT5
#define clr_ENF0  CAPCON2   &= ~SET_BIT4

//**** CKDIV	95H ****

//**** CKSWT	96H ****  TA protect register
/* SDCC some times uses extra instruction instead of straight 'orl','anl'
   so use asm here not to break TA protection
*/
#define set_HIRCST  do{SET_EA_TA;__asm__("orl _CKSWT,#0x20");EA=EA_SAVE;}while(0)
#define set_LIRCST  do{SET_EA_TA;__asm__("orl _CKSWT,#0x10");EA=EA_SAVE;}while(0)
#define set_ECLKST  do{SET_EA_TA;__asm__("orl _CKSWT,#0x08");EA=EA_SAVE;}while(0)
#define set_OSC1    do{SET_EA_TA;__asm__("orl _CKSWT,#0x04");EA=EA_SAVE;}while(0)
#define set_OSC0    do{SET_EA_TA;__asm__("orl _CKSWT,#0x02");EA=EA_SAVE;}while(0)

#define clr_HIRCST  do{SET_EA_TA;__asm__("anl _CKSWT,#0xDF");EA=EA_SAVE;}while(0)
#define clr_LIRCST  do{SET_EA_TA;__asm__("anl _CKSWT,#0xEF");EA=EA_SAVE;}while(0)
#define clr_ECLKST  do{SET_EA_TA;__asm__("anl _CKSWT,#0xF7");EA=EA_SAVE;}while(0)
#define clr_OSC1    do{SET_EA_TA;__asm__("anl _CKSWT,#0xFB");EA=EA_SAVE;}while(0)
#define clr_OSC0    do{SET_EA_TA;__asm__("anl _CKSWT,#0xFD");EA=EA_SAVE;}while(0)

//**** CKEN 	97H **** TA protect register
#define set_EXTEN1  do{SET_EA_TA;__asm__("orl _CKEN,#0x80");EA=EA_SAVE;}while(0)
#define set_EXTEN0  do{SET_EA_TA;__asm__("orl _CKEN,#0x40");EA=EA_SAVE;}while(0)
#define set_HIRCEN  do{SET_EA_TA;__asm__("orl _CKEN,#0x20");EA=EA_SAVE;}while(0)
#define set_CKSWTF  do{SET_EA_TA;__asm__("orl _CKEN,#0x01");EA=EA_SAVE;}while(0)

#define clr_EXTEN1  do{SET_EA_TA;__asm__("anl _CKEN,#0x7F");EA=EA_SAVE;}while(0)
#define clr_EXTEN0  do{SET_EA_TA;__asm__("anl _CKEN,#0xBF");EA=EA_SAVE;}while(0)
#define clr_HIRCEN  do{SET_EA_TA;__asm__("anl _CKEN,#0xDF");EA=EA_SAVE;}while(0)
#define clr_CKSWTF  do{SET_EA_TA;__asm__("anl _CKEN,#0xFE");EA=EA_SAVE;}while(0)

//**** SCON		98H ****
#define set_FE      FE  = 1
#define set_SM1     SM1 = 1
#define set_SM2     SM2 = 1
#define set_REN     REN = 1
#define set_TB8     TB8 = 1
#define set_RB8     RB8 = 1
#define set_TI      TI  = 1
#define set_RI      RI  = 1

#define clr_FE      FE  = 0
#define clr_SM1     SM1 = 0
#define clr_SM2     SM2 = 0
#define clr_REN     REN = 0
#define clr_TB8     TB8 = 0
#define clr_RB8     RB8 = 0
#define clr_TI      TI  = 0
#define clr_RI      RI  = 0

//**** SBUF		99H ****
//**** SBUF_1	9AH ****

//**** EIE		9BH ****
#define set_ET2     EIE |= SET_BIT7
#define set_ESPI    EIE |= SET_BIT6
#define set_EFB     EIE |= SET_BIT5
#define set_EWDT    EIE |= SET_BIT4
#define set_EPWM    EIE |= SET_BIT3
#define set_ECAP    EIE |= SET_BIT2
#define set_EPI     EIE |= SET_BIT1
#define set_EI2C    EIE |= SET_BIT0

#define clr_ET2     EIE &= ~SET_BIT7
#define clr_ESPI    EIE &= ~SET_BIT6
#define clr_EFB     EIE &= ~SET_BIT5
#define clr_EWDT    EIE &= ~SET_BIT4
#define clr_EPWM    EIE &= ~SET_BIT3
#define clr_ECAP    EIE &= ~SET_BIT2
#define clr_EPI     EIE &= ~SET_BIT1
#define clr_EI2C    EIE &= ~SET_BIT0

//**** CHPCON	9DH ****  TA protect register
// same as 'sw_reset()' from irq.h
#define set_SWRST   do{SET_EA_TA;__asm__("orl _CHPCON,#0x80");}while(0)
// does not make sense to set IAPFF from SW
//#define set_IAPFF   SET_EA_TA;CHPCON|=SET_BIT6;EA=EA_SAVE
#define set_BS      do{SET_EA_TA;__asm__("orl _CHPCON,#0x02");EA=EA_SAVE;}while(0)
// same as 'iap_enable()' from iap.h
#define set_IAPEN   do{SET_EA_TA;__asm__("orl _CHPCON,#0x01");EA=EA_SAVE;}while(0)

// cleared automatically by HW after reset is finished
// #define clr_SWRST   SET_EA_TA;CHPCON&=~SET_BIT7;EA=EA_SAVE
// same as 'iap_clear_error()' from iap.h
#define clr_IAPFF   do{SET_EA_TA;__asm__("anl _CHPCON,#0xbf");EA=EA_SAVE;}while(0)
#define clr_BS      do{SET_EA_TA;__asm__("anl _CHPCON,#0xfd");}while(0)
// same as 'iap_disable()' from iap.h
#define clr_IAPEN   do{SET_EA_TA;__asm__("anl _CHPCON,#0xfe");EA=EA_SAVE;}while(0)

//**** P2		A0H ****

//**** AUXR1	A2H ****
#define set_SWRF    AUXR1 |= SET_BIT7
#define set_RSTPINF AUXR1 |= SET_BIT6
#define set_HARDF  	AUXR1 |= SET_BIT5
#define set_GF2     AUXR1 |= SET_BIT3
#define set_UART0PX AUXR1 |= SET_BIT2
#define set_DPS     AUXR1 |= SET_BIT0

#define clr_SWRF    AUXR1 &= ~SET_BIT7
#define clr_RSTPINF AUXR1 &= ~SET_BIT6
#define clr_HARDF  	AUXR1 &= ~SET_BIT5
#define clr_GF2     AUXR1 &= ~SET_BIT3
#define clr_UART0PX AUXR1 &= ~SET_BIT2
#define clr_DPS     AUXR1 &= ~SET_BIT0

//**** BODCON0	A3H ****  TA protect register
#define set_BODEN   do{SET_EA_TA;__asm__("orl _BODCON0,#0x80");EA=EA_SAVE;}while(0)
#define set_BOV2    do{SET_EA_TA;__asm__("orl _BODCON0,#0x40");EA=EA_SAVE;}while(0)
#define set_BOV1    do{SET_EA_TA;__asm__("orl _BODCON0,#0x20");EA=EA_SAVE;}while(0)
#define set_BOV0    do{SET_EA_TA;__asm__("orl _BODCON0,#0x10");EA=EA_SAVE;}while(0)
#define set_BOF     do{SET_EA_TA;__asm__("orl _BODCON0,#0x08");EA=EA_SAVE;}while(0)
#define set_BORST   do{SET_EA_TA;__asm__("orl _BODCON0,#0x04");EA=EA_SAVE;}while(0)
#define set_BORF    do{SET_EA_TA;__asm__("orl _BODCON0,#0x02");EA=EA_SAVE;}while(0)
#define set_BOS     do{SET_EA_TA;__asm__("orl _BODCON0,#0x01");EA=EA_SAVE;}while(0)

#define clr_BODEN   do{SET_EA_TA;__asm__("anl _BODCON0,#0x7F");EA=EA_SAVE;}while(0)
#define clr_BOV2    do{SET_EA_TA;__asm__("anl _BODCON0,#0xBF");EA=EA_SAVE;}while(0)
#define clr_BOV1    do{SET_EA_TA;__asm__("anl _BODCON0,#0xDF");EA=EA_SAVE;}while(0)
#define clr_BOV0    do{SET_EA_TA;__asm__("anl _BODCON0,#0xEF");EA=EA_SAVE;}while(0)
#define clr_BOF     do{SET_EA_TA;__asm__("anl _BODCON0,#0xF7");EA=EA_SAVE;}while(0)
#define clr_BORST   do{SET_EA_TA;__asm__("anl _BODCON0,#0xFB");EA=EA_SAVE;}while(0)
#define clr_BORF    do{SET_EA_TA;__asm__("anl _BODCON0,#0xFD");EA=EA_SAVE;}while(0)
#define clr_BOS     do{SET_EA_TA;__asm__("anl _BODCON0,#0xFE");EA=EA_SAVE;}while(0)

//**** IAPTRG	A4H	****  TA protect register
// same as 'iap_trigger()' from iap.h
#define set_IAPGO   do{SET_EA_TA;__asm__("orl _IAPTRG,#0x01");EA=EA_SAVE;}while(0)

//**** IAPUEN	A5H **** TA protect register
// same as corresponding functions from iap.h
#define set_CFUEN   do{SET_EA_TA;__asm__("orl _IAPUEN,#0x04");EA=EA_SAVE;}while(0)
#define set_LDUEN   do{SET_EA_TA;__asm__("orl _IAPUEN,#0x02");EA=EA_SAVE;}while(0)
#define set_APUEN   do{SET_EA_TA;__asm__("orl _IAPUEN,#0x01");EA=EA_SAVE;}while(0)

#define clr_CFUEN   do{SET_EA_TA;__asm__("anl _IAPUEN,#0xFB");EA=EA_SAVE;}while(0)
#define clr_LDUEN   do{SET_EA_TA;__asm__("anl _IAPUEN,#0xFD");EA=EA_SAVE;}while(0)
#define clr_APUEN   do{SET_EA_TA;__asm__("anl _IAPUEN,#0xFE");EA=EA_SAVE;}while(0)

//**** IAPAL	A6H ****
//**** IAPAH	A7H ****

//**** IE		A8H ****
#define set_EA      EA   = 1
#define set_EADC    EADC = 1
#define set_EBOD    EBOD = 1
#define set_ES      ES   = 1
#define set_ET1     ET1  = 1
#define set_EX1     EX1  = 1
#define set_ET0     ET0  = 1
#define set_EX0     EX0  = 1

#define clr_EA      EA   = 0
#define clr_EADC    EADC = 0
#define clr_EBOD    EBOD = 0
#define clr_ES      ES   = 0
#define clr_ET1     ET1  = 0
#define clr_EX1     EX1  = 0
#define clr_ET0     ET0  = 0
#define clr_EX0     EX0  = 0

//**** SADDR	A9H ****

//**** WDCON	AAH **** TA protect register
#define set_WDTR   	do{SET_EA_TA;__asm__("orl _WDCON,#0x80");EA=EA_SAVE;}while(0)
#define set_WDCLR   do{SET_EA_TA;__asm__("orl _WDCON,#0x40");EA=EA_SAVE;}while(0)
#define set_WDTF    do{SET_EA_TA;__asm__("orl _WDCON,#0x20");EA=EA_SAVE;}while(0)
#define set_WIDPD	do{SET_EA_TA;__asm__("orl _WDCON,#0x10");EA=EA_SAVE;}while(0)
#define set_WDTRF   do{SET_EA_TA;__asm__("orl _WDCON,#0x08");EA=EA_SAVE;}while(0)
#define set_WPS2    do{SET_EA_TA;__asm__("orl _WDCON,#0x04");EA=EA_SAVE;}while(0)
#define set_WPS1    do{SET_EA_TA;__asm__("orl _WDCON,#0x02");EA=EA_SAVE;}while(0)
#define set_WPS0    do{SET_EA_TA;__asm__("orl _WDCON,#0x01");EA=EA_SAVE;}while(0)

#define clr_WDTEN   do{SET_EA_TA;__asm__("anl _WDCON,#0x7F");EA=EA_SAVE;}while(0)
#define clr_WDCLR   do{SET_EA_TA;__asm__("anl _WDCON,#0xBF");EA=EA_SAVE;}while(0)
#define clr_WDTF    do{SET_EA_TA;__asm__("anl _WDCON,#0xDF");EA=EA_SAVE;}while(0)
#define clr_WDTRF   do{SET_EA_TA;__asm__("anl _WDCON,#0xF7");EA=EA_SAVE;}while(0)
#define clr_WPS2    do{SET_EA_TA;__asm__("anl _WDCON,#0xFB");EA=EA_SAVE;}while(0)
#define clr_WPS1    do{SET_EA_TA;__asm__("anl _WDCON,#0xFD");EA=EA_SAVE;}while(0)
#define clr_WPS0    do{SET_EA_TA;__asm__("anl _WDCON,#0xFE");EA=EA_SAVE;}while(0)

//**** BODCON1 ABH **** TA protect register
#define set_LPBOD1  do{SET_EA_TA;__asm__("orl _BODCON1,#0x04");EA=EA_SAVE;}while(0)
#define set_LPBOD0  do{SET_EA_TA;__asm__("orl _BODCON1,#0x02");EA=EA_SAVE;}while(0)
#define set_BODFLT  do{SET_EA_TA;__asm__("orl _BODCON1,#0x01");EA=EA_SAVE;}while(0)

#define clr_LPBOD1  do{SET_EA_TA;__asm__("anl _BODCON1,#0xFB");EA=EA_SAVE;}while(0)
#define clr_LPBOD0  do{SET_EA_TA;__asm__("anl _BODCON1,#0xFD");EA=EA_SAVE;}while(0)
#define clr_BODFLT  do{SET_EA_TA;__asm__("anl _BODCON1,#0xFE");EA=EA_SAVE;}while(0)

//**** P3M1		ACH PAGE0 ****
#define set_P3M1_0  P3M1 |= SET_BIT0
#define clr_P3M1_0  P3M1 &= ~SET_BIT0

//**** P3S		ACH PAGE1 **** SFRS must set as 1 to modify this register
#define set_P3S_0   do{SET_EA_TA;SFRS=0x01;P3S|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P3S_0   do{SET_EA_TA;SFRS=0x01;P3S&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** P3M2		ADH PAGE0 ****
#define set_P3M2_0  P3M2 |= SET_BIT0
#define clr_P3M2_0  P3M2 &= ~SET_BIT0

//**** P3SR		ADH PAGE1 **** SFRS must set as 1 to modify this register
#define set_P3SR_0  do{SET_EA_TA;SFRS=0x01;P3SR|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P3SR_0  do{SET_EA_TA;SFRS=0x01;P3SR&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** IAPFD	AEH ****

//**** IAPCN	AFH ****
#define set_FOEN    IAPN |= SET_BIT5
#define set_FCEN    IAPN |= SET_BIT4
#define set_FCTRL3  IAPN |= SET_BIT3
#define set_FCTRL2  IAPN |= SET_BIT2
#define set_FCTRL1  IAPN |= SET_BIT1
#define set_FCTRL0  IAPN |= SET_BIT0

#define clr_FOEN    IAPN &= ~SET_BIT5
#define clr_FCEN    IAPN &= ~SET_BIT4
#define clr_FCTRL3  IAPN &= ~SET_BIT3
#define clr_FCTRL2  IAPN &= ~SET_BIT2
#define clr_FCTRL1  IAPN &= ~SET_BIT1
#define clr_FCTRL0  IAPN &= ~SET_BIT0

//**** P3		B0H ****
#define set_P30     P30      = 1
#define clr_P30     P30      = 0

//**** P0M1		B1H PAGE0 ****
#define set_P0M1_7  P0M1 |= SET_BIT7
#define set_P0M1_6  P0M1 |= SET_BIT6
#define set_P0M1_5  P0M1 |= SET_BIT5
#define set_P0M1_4  P0M1 |= SET_BIT4
#define set_P0M1_3  P0M1 |= SET_BIT3
#define set_P0M1_2  P0M1 |= SET_BIT2
#define set_P0M1_1  P0M1 |= SET_BIT1
#define set_P0M1_0  P0M1 |= SET_BIT0

#define clr_P0M1_7  P0M1 &= ~SET_BIT7
#define clr_P0M1_6  P0M1 &= ~SET_BIT6
#define clr_P0M1_5  P0M1 &= ~SET_BIT5
#define clr_P0M1_4  P0M1 &= ~SET_BIT4
#define clr_P0M1_3  P0M1 &= ~SET_BIT3
#define clr_P0M1_2  P0M1 &= ~SET_BIT2
#define clr_P0M1_1  P0M1 &= ~SET_BIT1
#define clr_P0M1_0  P0M1 &= ~SET_BIT0

//**** P0S		B2H PAGE1 **** SFRS must set as 1 to modify this register
#define set_P0S_7   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_6   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_5   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_4   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_3   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_2   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_1   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0S_0   do{SET_EA_TA;SFRS=0x01;P0S|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_P0S_7   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_6   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_5   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_4   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_3   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_2   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_1   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0S_0   do{SET_EA_TA;SFRS=0x01;P0S&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** P0M2		B2H PAGE0 ****
#define set_P0M2_7  P0M2 |= SET_BIT7
#define set_P0M2_6  P0M2 |= SET_BIT6
#define set_P0M2_5  P0M2 |= SET_BIT5
#define set_P0M2_4  P0M2 |= SET_BIT4
#define set_P0M2_3  P0M2 |= SET_BIT3
#define set_P0M2_2  P0M2 |= SET_BIT2
#define set_P0M2_1  P0M2 |= SET_BIT1
#define set_P0M2_0  P0M2 |= SET_BIT0

#define clr_P0M2_7  P0M2 &= ~SET_BIT7
#define clr_P0M2_6  P0M2 &= ~SET_BIT6
#define clr_P0M2_5  P0M2 &= ~SET_BIT5
#define clr_P0M2_4  P0M2 &= ~SET_BIT4
#define clr_P0M2_3  P0M2 &= ~SET_BIT3
#define clr_P0M2_2  P0M2 &= ~SET_BIT2
#define clr_P0M2_1  P0M2 &= ~SET_BIT1
#define clr_P0M2_0  P0M2 &= ~SET_BIT0

//**** P0SR		B0H PAGE1 **** SFRS must set as 1 to modify this register
#define set_P0SR_7  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_6  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_5  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_4  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_3  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_2  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_1  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P0SR_0  do{SET_EA_TA;SFRS=0x01;P0SR|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_P0SR_7  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_6  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_5  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_4  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_3  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_2  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_1  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P0SR_0  do{SET_EA_TA;SFRS=0x01;P0SR&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** P1M1		B3H PAGE0 ****
#define set_P1M1_7  P1M1 |= SET_BIT7
#define set_P1M1_6  P1M1 |= SET_BIT6
#define set_P1M1_5  P1M1 |= SET_BIT5
#define set_P1M1_4  P1M1 |= SET_BIT4
#define set_P1M1_3  P1M1 |= SET_BIT3
#define set_P1M1_2  P1M1 |= SET_BIT2
#define set_P1M1_1  P1M1 |= SET_BIT1
#define set_P1M1_0  P1M1 |= SET_BIT0

#define clr_P1M1_7  P1M1 &= ~SET_BIT7
#define clr_P1M1_6  P1M1 &= ~SET_BIT6
#define clr_P1M1_5  P1M1 &= ~SET_BIT5
#define clr_P1M1_4  P1M1 &= ~SET_BIT4
#define clr_P1M1_3  P1M1 &= ~SET_BIT3
#define clr_P1M1_2  P1M1 &= ~SET_BIT2
#define clr_P1M1_1  P1M1 &= ~SET_BIT1
#define clr_P1M1_0  P1M1 &= ~SET_BIT0

//**** P1S		B3H PAGE1 **** SFRS must set as 1 to modify this register
#define set_P1S_7	do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_6	do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_5	do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_4	do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_3	do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_2   do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_1   do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1S_0   do{SET_EA_TA;SFRS=0x01;P1S|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_P1S_7	do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_6	do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_5	do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_4	do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_3	do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_2   do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_1   do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1S_0   do{SET_EA_TA;SFRS=0x01;P1S&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** P1M2		B4H PAGE0 ****
#define set_P12UP   P1M2 |= SET_BIT2
#define set_P1M2_1  P1M2 |= SET_BIT1
#define set_P1M2_0  P1M2 |= SET_BIT0

#define clr_P12UP   P1M2 &= ~SET_BIT2
#define clr_P1M2_1  P1M2 &= ~SET_BIT1
#define clr_P1M2_0  P1M2 &= ~SET_BIT0

//**** P1SR		B4H PAGE1 **** SFRS must set as 1 to modify this register
#define set_P1SR_7	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_6	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_5	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_4	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_3	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_2	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_1	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_P1SR_0	do{SET_EA_TA;SFRS=0x01;P1SR|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_P1SR_7	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_6	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT6;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_5	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_4	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_3	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_2	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_1	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_P1SR_0	do{SET_EA_TA;SFRS=0x01;P1SR&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** P2S		B5H	****
#define set_P2S_0   P2S |= SET_BIT0
#define clr_P2S_0   P2S &= ~SET_BIT0

//**** IPH		B7H PAGE0 ****
#define set_PADCH   IPH	|= SET_BIT6
#define set_PBODH   IPH	|= SET_BIT5
#define set_PSH     IPH	|= SET_BIT4
#define set_PT1H    IPH	|= SET_BIT3
#define set_PX11    IPH	|= SET_BIT2
#define set_PT0H    IPH	|= SET_BIT1
#define set_PX0H    IPH	|= SET_BIT0

#define clr_PADCH   IPH	&= ~SET_BIT6
#define clr_PBODH   IPH	&= ~SET_BIT5
#define clr_PSH     IPH	&= ~SET_BIT4
#define clr_PT1H    IPH	&= ~SET_BIT3
#define clr_PX11    IPH	&= ~SET_BIT2
#define clr_PT0H    IPH	&= ~SET_BIT1
#define clr_PX0H    IPH	&= ~SET_BIT0

//**** PWMINTC	B7H PAGE1 **** SFRS must set as 1 to modify this register
#define set_INTTYP1	do{SET_EA_TA;SFRS=0x01;PWMINTC|=SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_INTTYP0	do{SET_EA_TA;SFRS=0x01;PWMINTC|=SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_INTSEL2	do{SET_EA_TA;SFRS=0x01;PWMINTC|=SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_INTSEL1	do{SET_EA_TA;SFRS=0x01;PWMINTC|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_INTSEL0	do{SET_EA_TA;SFRS=0x01;PWMINTC|=SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_INTTYP1 do{SET_EA_TA;SFRS=0x01;PWMINTC&=~SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_INTTYP0 do{SET_EA_TA;SFRS=0x01;PWMINTC&=~SET_BIT4;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_INTSEL2	do{SET_EA_TA;SFRS=0x01;PWMINTC&=~SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_INTSEL1	do{SET_EA_TA;SFRS=0x01;PWMINTC&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_INTSEL0	do{SET_EA_TA;SFRS=0x01;PWMINTC&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** IP		B8H	****
#define set_PADC    PADC = 1
#define set_PBOD    PBOD = 1
#define set_PS      PS   = 1
#define set_PT1     PT1  = 1
#define set_PX1     PX1  = 1
#define set_PT0     PT0  = 1
#define set_PX0     PX0  = 1

#define clr_PADC    PADC = 0
#define clr_PBOD    PBOD = 0
#define clr_PS      PS   = 0
#define clr_PT1     PT1  = 0
#define clr_PX1     PX1  = 0
#define clr_PT0     PT0  = 0
#define clr_PX0     PX0  = 0

//**** SADEN		B9H ****
//**** SADEN_1		8AH ****
//**** SADDR_1		BBH ****
//**** I2DAT		BCH ****
//**** I2STAT		BDH ****
//**** I2CLK		BEH ****

//**** I2TOC		BFH ****
#define set_I2TOCEN	I2TOC |= SET_BIT2
#define set_DIV		I2TOC |= SET_BIT1
#define set_I2TOF	I2TOC |= SET_BIT0

#define clr_I2TOCEN	I2TOC &= ~SET_BIT2
#define clr_DIV		I2TOC &= ~SET_BIT1
#define clr_I2TOF	I2TOC &= ~SET_BIT0

//**** I2CON	C0H ****
#define set_I2CEN   I2CEN	= 1
#define set_STA     STA		= 1
#define set_STO		STO		= 1
#define set_SI		SI		= 1
#define set_AA		AA		= 1
#define set_I2CPX	I2CPX	= 1

#define clr_I2CEN	I2CEN	= 0
#define clr_STA		STA		= 0
#define clr_STO		STO		= 0
#define clr_SI		SI		= 0
#define clr_AA		AA		= 0
#define clr_I2CPX	I2CPX	= 0

//**** I2ADDR	C1H ****
#define set_GC      I2ADDR  |= SET_BIT0
#define clr_GC      I2ADDR  &= ~SET_BIT0

//**** ADCRL	C2H ****
//**** ADCRH	C3H ****

//**** T3CON	C4H	PAGE0 ****
#define set_SMOD_1  T3CON |= SET_BIT7
#define set_SMOD0_1 T3CON |= SET_BIT6
#define set_BRCK    T3CON |= SET_BIT5
#define set_TF3     T3CON |= SET_BIT4
#define set_TR3     T3CON |= SET_BIT3
#define set_T3PS2   T3CON |= SET_BIT2
#define set_T3PS1   T3CON |= SET_BIT1
#define set_T3PS0   T3CON |= SET_BIT0

#define clr_SMOD_1  T3CON &= ~SET_BIT7
#define clr_SMOD0_1 T3CON &= ~SET_BIT6
#define clr_BRCK    T3CON &= ~SET_BIT5
#define clr_TF3     T3CON &= ~SET_BIT4
#define clr_TR3     T3CON &= ~SET_BIT3
#define clr_T3PS2   T3CON &= ~SET_BIT2
#define clr_T3PS1   T3CON &= ~SET_BIT1
#define clr_T3PS0   T3CON &= ~SET_BIT0

//**** PWM4H	C4H	PAGE1 **** SFRS must set as 1 to modify this register
//**** RL3		C5H PAGE0 ****
//**** PWM5H	C5H PAGE1 **** SFRS must set as 1 to modify this register
//**** RH3		C6H PAGE0 ****

//**** PIOCON1	C6H PAGE1 **** SFRS must set as 1 to modify this register
#define set_PIO15	do{SET_EA_TA;SFRS=0x01;PIOCON1|=SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_PIO13   do{SET_EA_TA;SFRS=0x01;PIOCON1|=SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_PIO12   do{SET_EA_TA;SFRS=0x01;PIOCON1|=SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_PIO11   do{SET_EA_TA;SFRS=0x01;PIOCON1|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_PIO15	do{SET_EA_TA;SFRS=0x01;PIOCON1&=~SET_BIT5;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_PIO13	do{SET_EA_TA;SFRS=0x01;PIOCON1&=~SET_BIT3;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_PIO12	do{SET_EA_TA;SFRS=0x01;PIOCON1&=~SET_BIT2;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_PIO11	do{SET_EA_TA;SFRS=0x01;PIOCON1&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** T2CON	C8H ****
#define set_TF2     TF2   = 1
#define set_TR2     TR2   = 1
#define set_CMRL2   CMRL2 = 1

#define clr_TF2     TF2   = 0
#define clr_TR2     TR2   = 0
#define clr_CMRL2   CMRL2 = 0

//**** T2MOD	C9H ****
#define set_LDEN    T2MOD |= SET_BIT7
#define set_T2DIV2  T2MOD |= SET_BIT6
#define set_T2DIV1  T2MOD |= SET_BIT5
#define set_T2DIV0  T2MOD |= SET_BIT4
#define set_CAPCR   T2MOD |= SET_BIT3
#define set_CMPCR   T2MOD |= SET_BIT2
#define set_LDTS1   T2MOD |= SET_BIT1
#define set_LDTS0   T2MOD |= SET_BIT0

#define clr_LDEN    T2MOD &= ~SET_BIT7
#define clr_T2DIV2  T2MOD &= ~SET_BIT6
#define clr_T2DIV1  T2MOD &= ~SET_BIT5
#define clr_T2DIV0  T2MOD &= ~SET_BIT4
#define clr_CAPCR   T2MOD &= ~SET_BIT3
#define clr_CMPCR   T2MOD &= ~SET_BIT2
#define clr_LDTS1   T2MOD &= ~SET_BIT1
#define clr_LDTS0   T2MOD &= ~SET_BIT0

//**** RCMP2H	CAH ****
//**** RCMP2L	CBH ****
//**** TL2		CCH PAGE0 ****
//**** PWM4L 	CCH PAGE1 **** SFRS must set as 1 to modify this register
//**** TH2		CDH PAGE0 ****
//**** PWM5L	CDH PAGE1 **** SFRS must set as 1 to modify this register
//**** ADCMPL	CEH ****
//**** ADCMPH	CFH ****

/****  PSW 		D0H ****/
#define set_CY	CY	= 1
#define set_AC	AC  = 1
#define set_F0	F0	= 1
#define set_RS1	RS1 = 1
#define set_RS0	RS0 = 1
#define set_OV	OV 	= 1
#define set_P	P	= 1

#define clr_CY	CY	= 0
#define clr_AC	AC  = 0
#define clr_F0	F0	= 0
#define clr_RS1	RS1 = 0
#define clr_RS0	RS0 = 0
#define clr_OV	OV 	= 0
#define clr_P	P	= 0

//**** PWMPH	D1H ****
//**** PWM0H	D2H ****
//**** PWM1H	D3H ****
//**** PWM2H	D4H ****
//**** PWM3H	D5H	****

//**** PNP		D6H ****
#define set_PNP5    PNP |= SET_BIT5
#define set_PNP4    PNP |= SET_BIT4
#define set_PNP3    PNP |= SET_BIT3
#define set_PNP2    PNP |= SET_BIT2
#define set_PNP1    PNP |= SET_BIT1
#define set_PNP0    PNP |= SET_BIT0

#define clr_PNP5    PNP &= ~SET_BIT5
#define clr_PNP4    PNP &= ~SET_BIT4
#define clr_PNP3    PNP &= ~SET_BIT3
#define clr_PNP2    PNP &= ~SET_BIT2
#define clr_PNP1    PNP &= ~SET_BIT1
#define clr_PNP0    PNP &= ~SET_BIT0

//**** FBD		D7H ****
#define set_FBF     FBD |= SET_BIT7
#define set_FBINLS  FBD |= SET_BIT6
#define set_FBD5    FBD |= SET_BIT5
#define set_FBD4    FBD |= SET_BIT4
#define set_FBD3    FBD |= SET_BIT3
#define set_FBD2    FBD |= SET_BIT2
#define set_FBD1    FBD |= SET_BIT1
#define set_FBD0    FBD |= SET_BIT0

#define clr_FBF     FBD &= ~SET_BIT7
#define clr_FBINLS  FBD &= ~SET_BIT6
#define clr_FBD5    FBD &= ~SET_BIT5
#define clr_FBD4    FBD &= ~SET_BIT4
#define clr_FBD3    FBD &= ~SET_BIT3
#define clr_FBD2    FBD &= ~SET_BIT2
#define clr_FBD1    FBD &= ~SET_BIT1
#define clr_FBD0    FBD &= ~SET_BIT0

/**** PWMCON0	D8H ****/
#define set_PWMRUN  PWMRUN = 1
#define set_LOAD    LOAD   = 1
#define set_PWMF    PWMF   = 1
#define set_CLRPWM  CLRPWM = 1

#define clr_PWMRUN  PWMRUN = 0
#define clr_LOAD    LOAD   = 0
#define clr_PWMF    PWMF   = 0
#define clr_CLRPWM  CLRPWM = 0

//**** PWMPL	D9H ****
//**** PWM0L	DAH ****
//**** PWM1L	DBH ****
//**** PWM2L	DCH ****
//**** PWM3L	DDH ****

//**** PIOCON0	DEH ****
#define set_PIO05    PIOCON0 |= SET_BIT5
#define set_PIO04    PIOCON0 |= SET_BIT4
#define set_PIO03    PIOCON0 |= SET_BIT3
#define set_PIO02    PIOCON0 |= SET_BIT2
#define set_PIO01    PIOCON0 |= SET_BIT1
#define set_PIO00    PIOCON0 |= SET_BIT0

#define clr_PIO05    PIOCON0 &= ~SET_BIT5
#define clr_PIO04    PIOCON0 &= ~SET_BIT4
#define clr_PIO03    PIOCON0 &= ~SET_BIT3
#define clr_PIO02    PIOCON0 &= ~SET_BIT2
#define clr_PIO01    PIOCON0 &= ~SET_BIT1
#define clr_PIO00    PIOCON0 &= ~SET_BIT0

//**** PWMCON1	DFH ****
#define set_PWMMOD1		PWMCON1 |= SET_BIT7
#define set_PWMMOD0		PWMCON1 |= SET_BIT6
#define set_GP      	PWMCON1 |= SET_BIT5
#define set_PWMTYP  	PWMCON1 |= SET_BIT4
#define set_FBINEN		PWMCON1 |= SET_BIT3
#define set_PWMDIV2		PWMCON1 |= SET_BIT2
#define set_PWMDIV1		PWMCON1 |= SET_BIT1
#define set_PWMDIV0		PWMCON1 |= SET_BIT0

#define clr_PWMMOD1		PWMCON1 &= ~SET_BIT7
#define clr_PWMMOD0		PWMCON1 &= ~SET_BIT6
#define clr_GP			PWMCON1 &= ~SET_BIT5
#define clr_PWMTYP		PWMCON1 &= ~SET_BIT4
#define clr_FBINEN		PWMCON1 &= ~SET_BIT3
#define clr_PWMDIV2		PWMCON1 &= ~SET_BIT2
#define clr_PWMDIV1		PWMCON1 &= ~SET_BIT1
#define clr_PWMDIV0		PWMCON1 &= ~SET_BIT0

//**** ACC		E0H ****

//**** ADCCON1	E1H ****
#define set_STADCPX ADCCON1 |= SET_BIT6
#define set_ETGTYP1 ADCCON1 |= SET_BIT3
#define set_ETGTYP0 ADCCON1 |= SET_BIT2
#define set_ADCEX   ADCCON1 |= SET_BIT1
#define set_ADCEN   ADCCON1 |= SET_BIT0

#define clr_STADCPX ADCCON1 &= ~SET_BIT6
#define clr_ETGTYP1 ADCCON1 &= ~SET_BIT3
#define clr_ETGTYP0 ADCCON1 &= ~SET_BIT2
#define clr_ADCEX   ADCCON1 &= ~SET_BIT1
#define clr_ADCEN   ADCCON1 &= ~SET_BIT0

//**** ADCON2	E2H ****
#define set_ADFBEN  ADCCON2 |= SET_BIT7
#define set_ADCMPOP ADCCON2 |= SET_BIT6
#define set_ADCMPEN ADCCON2 |= SET_BIT5
#define set_ADCMPO  ADCCON2 |= SET_BIT4

#define clr_ADFBEN  ADCCON2 &= ~SET_BIT7
#define clr_ADCMPOP ADCCON2 &= ~SET_BIT6
#define clr_ADCMPEN ADCCON2 &= ~SET_BIT5
#define clr_ADCMPO  ADCCON2 &= ~SET_BIT4

//**** ADCDLY	E3H ****
//**** C0L		E4H ****
//**** C0H		E5H ****
//**** C1L		E6H ****
//**** C1H		E7H ****

//**** ADCCON0	EAH ****
#define set_ADCF    ADCF     = 1
#define set_ADCS    ADCS     = 1
#define set_ETGSEL1 ETGSEL1  = 1
#define set_ETGSEL0 ETGSEL0  = 1
#define set_ADCHS3  ADCHS3   = 1
#define set_ADCHS2  ADCHS2   = 1
#define set_ADCHS1  ADCHS1   = 1
#define set_ADCHS0  ADCHS0   = 1

#define clr_ADCF    ADCF    = 0
#define clr_ADCS    ADCS    = 0
#define clr_ETGSEL1 ETGSEL1 = 0
#define clr_ETGSEL0 ETGSEL0 = 0
#define clr_ADCHS3  ADCHS3  = 0
#define clr_ADCHS2  ADCHS2  = 0
#define clr_ADCHS1  ADCHS1  = 0
#define clr_ADCHS0  ADCHS0  = 0

//**** PICON	E9H	****
#define set_PIT67   PICON |= SET_BIT7
#define set_PIT45   PICON |= SET_BIT6
#define set_PIT3    PICON |= SET_BIT5
#define set_PIT2    PICON |= SET_BIT4
#define set_PIT1    PICON |= SET_BIT3
#define set_PIT0    PICON |= SET_BIT2
#define set_PIPS1   PICON |= SET_BIT1
#define set_PIPS0   PICON |= SET_BIT0

#define clr_PIT67   PICON &= ~SET_BIT7
#define clr_PIT45   PICON &= ~SET_BIT6
#define clr_PIT3    PICON &= ~SET_BIT5
#define clr_PIT2    PICON &= ~SET_BIT4
#define clr_PIT1    PICON &= ~SET_BIT3
#define clr_PIT0    PICON &= ~SET_BIT2
#define clr_PIPS1   PICON &= ~SET_BIT1
#define clr_PIPS0   PICON &= ~SET_BIT0

//**** PINEN	EAH ****
#define set_PINEN7  PINEN |= SET_BIT7
#define set_PINEN6  PINEN |= SET_BIT6
#define set_PINEN5  PINEN |= SET_BIT5
#define set_PINEN4  PINEN |= SET_BIT4
#define set_PINEN3  PINEN |= SET_BIT3
#define set_PINEN2  PINEN |= SET_BIT2
#define set_PINEN1  PINEN |= SET_BIT1
#define set_PINEN0  PINEN |= SET_BIT0

#define clr_PINEN7  PINEN &= ~SET_BIT7
#define clr_PINEN6  PINEN &= ~SET_BIT6
#define clr_PINEN5  PINEN &= ~SET_BIT5
#define clr_PINEN4  PINEN &= ~SET_BIT4
#define clr_PINEN3  PINEN &= ~SET_BIT3
#define clr_PINEN2  PINEN &= ~SET_BIT2
#define clr_PINEN1  PINEN &= ~SET_BIT1
#define clr_PINEN0  PINEN &= ~SET_BIT0

//**** PIPEN 	EBH ****
#define set_PIPEN7  PIPEN |= SET_BIT7
#define set_PIPEN6  PIPEN |= SET_BIT6
#define set_PIPEN5  PIPEN |= SET_BIT5
#define set_PIPEN4  PIPEN |= SET_BIT4
#define set_PIPEN3  PIPEN |= SET_BIT3
#define set_PIPEN2  PIPEN |= SET_BIT2
#define set_PIPEN1  PIPEN |= SET_BIT1
#define set_PIPEN0  PIPEN |= SET_BIT0

#define clr_PIPEN7  PIPEN &= ~SET_BIT7
#define clr_PIPEN6  PIPEN &= ~SET_BIT6
#define clr_PIPEN5  PIPEN &= ~SET_BIT5
#define clr_PIPEN4  PIPEN &= ~SET_BIT4
#define clr_PIPEN3  PIPEN &= ~SET_BIT3
#define clr_PIPEN2  PIPEN &= ~SET_BIT2
#define clr_PIPEN1  PIPEN &= ~SET_BIT1
#define clr_PIPEN0  PIPEN &= ~SET_BIT0

//**** PIF		ECH ****
#define set_PIF7    PIF |= SET_BIT7
#define set_PIF6    PIF |= SET_BIT6
#define set_PIF5    PIF |= SET_BIT5
#define set_PIF4    PIF |= SET_BIT4
#define set_PIF3    PIF |= SET_BIT3
#define set_PIF2    PIF |= SET_BIT2
#define set_PIF1    PIF |= SET_BIT1
#define set_PIF0    PIF |= SET_BIT0

#define clr_PIF7    PIF &= ~SET_BIT7
#define clr_PIF6    PIF &= ~SET_BIT6
#define clr_PIF5    PIF &= ~SET_BIT5
#define clr_PIF4    PIF &= ~SET_BIT4
#define clr_PIF3    PIF &= ~SET_BIT3
#define clr_PIF2    PIF &= ~SET_BIT2
#define clr_PIF1    PIF &= ~SET_BIT1
#define clr_PIF0    PIF &= ~SET_BIT0

//**** C2L		EDH ****
//**** C2H		EEH ****

//**** EIP		EFH ****
#define set_PT2     EIP |= SET_BIT7
#define set_PSPI    EIP |= SET_BIT6
#define set_PFB     EIP |= SET_BIT5
#define set_PWDT    EIP |= SET_BIT4
#define set_PPWM    EIP |= SET_BIT3
#define set_PCAP    EIP |= SET_BIT2
#define set_PPI     EIP |= SET_BIT1
#define set_PI2C    EIP |= SET_BIT0

#define clr_PT2     EIP &= ~SET_BIT7
#define clr_PSPI    EIP &= ~SET_BIT6
#define clr_PFB     EIP &= ~SET_BIT5
#define clr_PWDT    EIP &= ~SET_BIT4
#define clr_PPWM    EIP &= ~SET_BIT3
#define clr_PCAP    EIP &= ~SET_BIT2
#define clr_PPI     EIP &= ~SET_BIT1
#define clr_PI2C    EIP &= ~SET_BIT0

//**** B		F0H ****

//**** CAPCON3	F1H ****
#define set_CAP13   CAPCON3 |= SET_BIT7
#define set_CAP12   CAPCON3 |= SET_BIT6
#define set_CAP11   CAPCON3 |= SET_BIT5
#define set_CAP10   CAPCON3 |= SET_BIT4
#define set_CAP03   CAPCON3 |= SET_BIT3
#define set_CAP02   CAPCON3 |= SET_BIT2
#define set_CAP01   CAPCON3 |= SET_BIT1
#define set_CAP00   CAPCON3 |= SET_BIT0

#define clr_CAP13   CAPCON3 &= ~SET_BIT7
#define clr_CAP12   CAPCON3 &= ~SET_BIT6
#define clr_CAP11   CAPCON3 &= ~SET_BIT5
#define clr_CAP10   CAPCON3 &= ~SET_BIT4
#define clr_CAP03   CAPCON3 &= ~SET_BIT3
#define clr_CAP02   CAPCON3 &= ~SET_BIT2
#define clr_CAP01   CAPCON3 &= ~SET_BIT1
#define clr_CAP00   CAPCON3 &= ~SET_BIT0

//**** CAPCON4	F2H ****
#define set_CAP23    CAPCON4 |= SET_BIT3
#define set_CAP22    CAPCON4 |= SET_BIT2
#define set_CAP21    CAPCON4 |= SET_BIT1
#define set_CAP20    CAPCON4 |= SET_BIT0

#define clr_CAP23    CAPCON4 &= ~SET_BIT3
#define clr_CAP22    CAPCON4 &= ~SET_BIT2
#define clr_CAP21    CAPCON4 &= ~SET_BIT1
#define clr_CAP20    CAPCON4 &= ~SET_BIT0

//**** SPCR		F3H PAGE0 ****
#define set_SSOE    SPCR |= SET_BIT7
#define set_SPIEN   SPCR |= SET_BIT6
#define set_LSBFE   SPCR |= SET_BIT5
#define set_MSTR    SPCR |= SET_BIT4
#define set_CPOL    SPCR |= SET_BIT3
#define set_CPHA    SPCR |= SET_BIT2
#define set_SPR1    SPCR |= SET_BIT1
#define set_SPR0    SPCR |= SET_BIT0

#define clr_SSOE    SPCR &= ~SET_BIT7
#define clr_SPIEN   SPCR &= ~SET_BIT6
#define clr_LSBFE   SPCR &= ~SET_BIT5
#define clr_MSTR    SPCR &= ~SET_BIT4
#define clr_CPOL    SPCR &= ~SET_BIT3
#define clr_CPHA    SPCR &= ~SET_BIT2
#define clr_SPR1    SPCR &= ~SET_BIT1
#define clr_SPR0    SPCR &= ~SET_BIT0

//**** SPCR2	F3H PAGE1 **** SFRS must set as 1 to modify this register
#define set_SPIS1	do{SET_EA_TA;SFRS=0x01;SPCR2|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define set_SPIS0	do{SET_EA_TA;SFRS=0x01;SPCR2|=SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

#define clr_SPIS1	do{SET_EA_TA;SFRS=0x01;SPCR2&=~SET_BIT1;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define clr_SPIS0	do{SET_EA_TA;SFRS=0x01;SPCR2&=~SET_BIT0;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)

//**** SPSR		F4H ****
#define set_SPIF    SPSR |= SET_BIT7
#define set_WCOL    SPSR |= SET_BIT6
#define set_SPIOVF  SPSR |= SET_BIT5
#define set_MODF    SPSR |= SET_BIT4
#define set_DISMODF SPSR |= SET_BIT3

#define clr_SPIF    SPSR &= ~SET_BIT7
#define clr_WCOL    SPSR &= ~SET_BIT6
#define clr_SPIOVF  SPSR &= ~SET_BIT5
#define clr_MODF    SPSR &= ~SET_BIT4
#define clr_DISMODF SPSR &= ~SET_BIT3

//**** SPDR		F5H ****

//**** AINDIDS	F6H ****
#define set_P11DIDS AINDIDS |= SET_BIT7
#define set_P03DIDS AINDIDS |= SET_BIT6
#define set_P04DIDS AINDIDS |= SET_BIT5
#define set_P05DIDS AINDIDS |= SET_BIT4
#define set_P06DIDS AINDIDS |= SET_BIT3
#define set_P07DIDS AINDIDS |= SET_BIT2
#define set_P30DIDS AINDIDS |= SET_BIT1
#define set_P17DIDS AINDIDS |= SET_BIT0

#define clr_P11DIDS AINDIDS &= ~SET_BIT7
#define clr_P03DIDS AINDIDS &= ~SET_BIT6
#define clr_P04DIDS AINDIDS &= ~SET_BIT5
#define clr_P05DIDS AINDIDS &= ~SET_BIT4
#define clr_P06DIDS AINDIDS &= ~SET_BIT3
#define clr_P07DIDS AINDIDS &= ~SET_BIT2
#define clr_P30DIDS AINDIDS &= ~SET_BIT1
#define clr_P17DIDS AINDIDS &= ~SET_BIT0

//**** EIPH		F7H ****
#define set_PT2H    EIPH |= SET_BIT7
#define set_PSPIH   EIPH |= SET_BIT6
#define set_PFBH    EIPH |= SET_BIT5
#define set_PWDTH   EIPH |= SET_BIT4
#define set_PPWMH   EIPH |= SET_BIT3
#define set_PCAPH   EIPH |= SET_BIT2
#define set_PPIH    EIPH |= SET_BIT1
#define set_PI2CH   EIPH |= SET_BIT0

#define clr_PT2H    EIPH &= ~SET_BIT7
#define clr_PSPIH   EIPH &= ~SET_BIT6
#define clr_PFBH    EIPH &= ~SET_BIT5
#define clr_PWDTH   EIPH &= ~SET_BIT4
#define clr_PPWMH   EIPH &= ~SET_BIT3
#define clr_PCAPH   EIPH &= ~SET_BIT2
#define clr_PPIH    EIPH &= ~SET_BIT1
#define clr_PI2CH   EIPH &= ~SET_BIT0

/**** SCON_1	F8H ****/
#define set_FE_1    FE_1  = 1
#define set_SM1_1   SM1_1 = 1
#define set_SM2_1   SM2_1 = 1
#define set_REN_1   REN_1 = 1
#define set_TB8_1   TB8_1 = 1
#define set_RB8_1   RB8_1 = 1
#define set_TI_1    TI_1  = 1
#define set_RI_1    RI_1  = 1

#define clr_FE_1    FE_1  = 0
#define clr_SM1_1   SM1_1 = 0
#define clr_SM2_1   SM2_1 = 0
#define clr_REN_1   REN_1 = 0
#define clr_TB8_1   TB8_1 = 0
#define clr_RB8_1   RB8_1 = 0
#define clr_TI_1    TI_1  = 0
#define clr_RI_1    RI_1  = 0

//**** PDTEN	F9H ****
#define set_PDT45EN do{SET_EA_TA;__asm__("orl _PDTEN,#0x04");EA=EA_SAVE;}while(0)
#define set_PDT23EN do{SET_EA_TA;__asm__("orl _PDTEN,#0x02");EA=EA_SAVE;}while(0)
#define set_PDT01EN do{SET_EA_TA;__asm__("orl _PDTEN,#0x01");EA=EA_SAVE;}while(0)

#define clr_PDT45EN do{SET_EA_TA;__asm__("anl _PDTEN,#0xFB");EA=EA_SAVE;}while(0)
#define clr_PDT23EN do{SET_EA_TA;__asm__("anl _PDTEN,#0xFD");EA=EA_SAVE;}while(0)
#define clr_PDT01EN do{SET_EA_TA;__asm__("anl _PDTEN,#0xFE");EA=EA_SAVE;}while(0)

//**** PDTCNT	FAH ****

//**** PMEN   	FBH ****
#define set_PMEN5   PMEN |= SET_BIT5
#define set_PMEN4   PMEN |= SET_BIT4
#define set_PMEN3   PMEN |= SET_BIT3
#define set_PMEN2   PMEN |= SET_BIT2
#define set_PMEN1   PMEN |= SET_BIT1
#define set_PMEN0   PMEN |= SET_BIT0

#define clr_PMEN5   PMEN &= ~SET_BIT5
#define clr_PMEN4   PMEN &= ~SET_BIT4
#define clr_PMEN3   PMEN &= ~SET_BIT3
#define clr_PMEN2   PMEN &= ~SET_BIT2
#define clr_PMEN1   PMEN &= ~SET_BIT1
#define clr_PMEN0   PMEN &= ~SET_BIT0

//**** PMD		FCH ****
#define set_PMD7    PMD  |= SET_BIT7
#define set_PMD6    PMD  |= SET_BIT6
#define set_PMD5    PMD  |= SET_BIT5
#define set_PMD4    PMD  |= SET_BIT4
#define set_PMD3    PMD  |= SET_BIT3
#define set_PMD2    PMD  |= SET_BIT2
#define set_PMD1    PMD  |= SET_BIT1
#define set_PMD0    PMD  |= SET_BIT0

#define clr_PMD7    PMD  &= ~SET_BIT7
#define clr_PMD6    PMD  &= ~SET_BIT6
#define clr_PMD5    PMD  &= ~SET_BIT5
#define clr_PMD4    PMD  &= ~SET_BIT4
#define clr_PMD3    PMD  &= ~SET_BIT3
#define clr_PMD2    PMD  &= ~SET_BIT2
#define clr_PMD1    PMD  &= ~SET_BIT1
#define clr_PMD0    PMD  &= ~SET_BIT0

//****	EIP1 	FEH ****
#define set_PWKT    EIP1 |= SET_BIT2
#define set_PT3     EIP1 |= SET_BIT1
#define set_PS_1    EIP1 |= SET_BIT0

#define clr_PWKT    EIP1 &= ~SET_BIT2
#define clr_PT3     EIP1 &= ~SET_BIT1
#define clr_PS_1    EIP1 &= ~SET_BIT0

//**** EIPH1	FFH ****
#define set_PWKTH   EIPH1 |= SET_BIT2
#define set_PT3H    EIPH1 |= SET_BIT1
#define set_PSH_1   EIPH1 |= SET_BIT0

#define clr_PWKTH   EIPH1 &= ~SET_BIT2
#define clr_PT3H    EIPH1 &= ~SET_BIT1
#define clr_PSH_1   EIPH1 &= ~SET_BIT0

//**** Functional macros from Function_define.h V1.02

// 16 --> 8 x 2
#define HIBYTE(v1) ((uint8_t)(((v1)>>8)&0xFF))   // v1 is uint16_t
#define LOBYTE(v1) ((uint8_t)((v1)&0xFF))
// 8 x 2 --> 16
#define MAKEWORD(v1,v2)	((((uint16_t)(v1))<<8)+(uint16_t)(v2)) // v1,v2 is uint8_t
// 8 x 4 --> 32
#define MAKELONG(v1,v2,v3,v4) (uint32_t)((v1<<32)+(v2<<16)+(v3<<8)+v4) // v1,v2,v3,v4 is uint8_t
// 32 --> 16 x 2
#define YBYTE1(v1) ((uint16_t)((v1)>>16))   // v1 is uint32_t
#define YBYTE0(v1) ((uint16_t)((v1)&0xFFFF))
// 32 --> 8 x 4
#define TBYTE3(v1) ((uint8_t)((v1)>>24))    // v1 is uint32_t
#define TBYTE2(v1) ((uint8_t)((v1)>>16))
#define TBYTE1(v1) ((uint8_t)((v1)>>8))
#define TBYTE0(v1) ((uint8_t)((v1)&0xFF))

#define SET_BIT0  0x01
#define SET_BIT1  0x02
#define SET_BIT2  0x04
#define SET_BIT3  0x08
#define SET_BIT4  0x10
#define SET_BIT5  0x20
#define SET_BIT6  0x40
#define SET_BIT7  0x80
#define SET_BIT8  0x0100
#define SET_BIT9  0x0200
#define SET_BIT10 0x0400
#define SET_BIT11 0x0800
#define SET_BIT12 0x1000
#define SET_BIT13 0x2000
#define SET_BIT14 0x4000
#define SET_BIT15 0x8000

#define CLR_BIT0  0xFE
#define CLR_BIT1  0xFD
#define CLR_BIT2  0xFB
#define CLR_BIT3  0xF7
#define CLR_BIT4  0xEF
#define CLR_BIT5  0xDF
#define CLR_BIT6  0xBF
#define CLR_BIT7  0x7F

#define CLR_BIT8  0xFEFF
#define CLR_BIT9  0xFDFF
#define CLR_BIT10 0xFBFF
#define CLR_BIT11 0xF7FF
#define CLR_BIT12 0xEFFF
#define CLR_BIT13 0xDFFF
#define CLR_BIT14 0xBFFF
#define CLR_BIT15 0x7FFF

#define FAIL      1
#define PASS      0

/*******************************************************************************
* For GPIO INIT setting
*******************************************************************************/
//------------------- Define Port as Quasi mode  -------------------
#define P00_Quasi_Mode		do{P0M1&=~SET_BIT0;P0M2&=~SET_BIT0;}while(0)
#define P01_Quasi_Mode		do{P0M1&=~SET_BIT1;P0M2&=~SET_BIT1;}while(0)
#define P02_Quasi_Mode		do{P0M1&=~SET_BIT2;P0M2&=~SET_BIT2;}while(0)
#define P03_Quasi_Mode		do{P0M1&=~SET_BIT3;P0M2&=~SET_BIT3;}while(0)
#define P04_Quasi_Mode		do{P0M1&=~SET_BIT4;P0M2&=~SET_BIT4;}while(0)
#define P05_Quasi_Mode		do{P0M1&=~SET_BIT5;P0M2&=~SET_BIT5;}while(0)
#define P06_Quasi_Mode		do{P0M1&=~SET_BIT6;P0M2&=~SET_BIT6;}while(0)
#define P07_Quasi_Mode		do{P0M1&=~SET_BIT7;P0M2&=~SET_BIT7;}while(0)
#define P10_Quasi_Mode		do{P1M1&=~SET_BIT0;P1M2&=~SET_BIT0;}while(0)
#define P11_Quasi_Mode		do{P1M1&=~SET_BIT1;P1M2&=~SET_BIT1;}while(0)
#define P12_Quasi_Mode		do{P1M1&=~SET_BIT2;P1M2&=~SET_BIT2;}while(0)
#define P13_Quasi_Mode		do{P1M1&=~SET_BIT3;P1M2&=~SET_BIT3;}while(0)
#define P14_Quasi_Mode		do{P1M1&=~SET_BIT4;P1M2&=~SET_BIT4;}while(0)
#define P15_Quasi_Mode		do{P1M1&=~SET_BIT5;P1M2&=~SET_BIT5;}while(0)
#define P16_Quasi_Mode		do{P1M1&=~SET_BIT6;P1M2&=~SET_BIT6;}while(0)
#define P17_Quasi_Mode		do{P1M1&=~SET_BIT7;P1M2&=~SET_BIT7;}while(0)
#define P30_Quasi_Mode		do{P3M1&=~SET_BIT0;P3M2&=~SET_BIT0;}while(0)
//------------------- Define Port as Push Pull mode -------------------
#define P00_PushPull_Mode	do{P0M1&=~SET_BIT0;P0M2|=SET_BIT0;}while(0)
#define P01_PushPull_Mode	do{P0M1&=~SET_BIT1;P0M2|=SET_BIT1;}while(0)
#define P02_PushPull_Mode	do{P0M1&=~SET_BIT2;P0M2|=SET_BIT2;}while(0)
#define P03_PushPull_Mode	do{P0M1&=~SET_BIT3;P0M2|=SET_BIT3;}while(0)
#define P04_PushPull_Mode	do{P0M1&=~SET_BIT4;P0M2|=SET_BIT4;}while(0)
#define P05_PushPull_Mode	do{P0M1&=~SET_BIT5;P0M2|=SET_BIT5;}while(0)
#define P06_PushPull_Mode	do{P0M1&=~SET_BIT6;P0M2|=SET_BIT6;}while(0)
#define P07_PushPull_Mode	do{P0M1&=~SET_BIT7;P0M2|=SET_BIT7;}while(0)
#define P10_PushPull_Mode	do{P1M1&=~SET_BIT0;P1M2|=SET_BIT0;}while(0)
#define P11_PushPull_Mode	do{P1M1&=~SET_BIT1;P1M2|=SET_BIT1;}while(0)
#define P12_PushPull_Mode	do{P1M1&=~SET_BIT2;P1M2|=SET_BIT2;}while(0)
#define P13_PushPull_Mode	do{P1M1&=~SET_BIT3;P1M2|=SET_BIT3;}while(0)
#define P14_PushPull_Mode	do{P1M1&=~SET_BIT4;P1M2|=SET_BIT4;}while(0)
#define P15_PushPull_Mode	do{P1M1&=~SET_BIT5;P1M2|=SET_BIT5;}while(0)
#define P16_PushPull_Mode	do{P1M1&=~SET_BIT6;P1M2|=SET_BIT6;}while(0)
#define P17_PushPull_Mode	do{P1M1&=~SET_BIT7;P1M2|=SET_BIT7;}while(0)
#define P30_PushPull_Mode	do{P3M1&=~SET_BIT0;P3M2|=SET_BIT0;}while(0)
#define GPIO1_PushPull_Mode	do{P1M1&=~SET_BIT0;P1M2|=SET_BIT0;}while(0)
//------------------- Define Port as Input Only mode -------------------
#define P00_Input_Mode		do{P0M1|=SET_BIT0;P0M2&=~SET_BIT0;}while(0)
#define P01_Input_Mode		do{P0M1|=SET_BIT1;P0M2&=~SET_BIT1;}while(0)
#define P02_Input_Mode		do{P0M1|=SET_BIT2;P0M2&=~SET_BIT2;}while(0)
#define P03_Input_Mode		do{P0M1|=SET_BIT3;P0M2&=~SET_BIT3;}while(0)
#define P04_Input_Mode		do{P0M1|=SET_BIT4;P0M2&=~SET_BIT4;}while(0)
#define P05_Input_Mode		do{P0M1|=SET_BIT5;P0M2&=~SET_BIT5;}while(0)
#define P06_Input_Mode		do{P0M1|=SET_BIT6;P0M2&=~SET_BIT6;}while(0)
#define P07_Input_Mode		do{P0M1|=SET_BIT7;P0M2&=~SET_BIT7;}while(0)
#define P10_Input_Mode		do{P1M1|=SET_BIT0;P1M2&=~SET_BIT0;}while(0)
#define P11_Input_Mode		do{P1M1|=SET_BIT1;P1M2&=~SET_BIT1;}while(0)
#define P12_Input_Mode		do{P1M1|=SET_BIT2;P1M2&=~SET_BIT2;}while(0)
#define P13_Input_Mode		do{P1M1|=SET_BIT3;P1M2&=~SET_BIT3;}while(0)
#define P14_Input_Mode		do{P1M1|=SET_BIT4;P1M2&=~SET_BIT4;}while(0)
#define P15_Input_Mode		do{P1M1|=SET_BIT5;P1M2&=~SET_BIT5;}while(0)
#define P16_Input_Mode		do{P1M1|=SET_BIT6;P1M2&=~SET_BIT6;}while(0)
#define P17_Input_Mode		do{P1M1|=SET_BIT7;P1M2&=~SET_BIT7;}while(0)
#define P30_Input_Mode		do{P3M1|=SET_BIT0;P3M2&=~SET_BIT0;}while(0)
//-------------------Define Port as Open Drain mode -------------------
#define P00_OpenDrain_Mode	do{P0M1|=SET_BIT0;P0M2|=SET_BIT0;}while(0)
#define P01_OpenDrain_Mode	do{P0M1|=SET_BIT1;P0M2|=SET_BIT1;}while(0)
#define P02_OpenDrain_Mode	do{P0M1|=SET_BIT2;P0M2|=SET_BIT2;}while(0)
#define P03_OpenDrain_Mode	do{P0M1|=SET_BIT3;P0M2|=SET_BIT3;}while(0)
#define P04_OpenDrain_Mode	do{P0M1|=SET_BIT4;P0M2|=SET_BIT4;}while(0)
#define P05_OpenDrain_Mode	do{P0M1|=SET_BIT5;P0M2|=SET_BIT5;}while(0)
#define P06_OpenDrain_Mode	do{P0M1|=SET_BIT6;P0M2|=SET_BIT6;}while(0)
#define P07_OpenDrain_Mode	do{P0M1|=SET_BIT7;P0M2|=SET_BIT7;}while(0)
#define P10_OpenDrain_Mode	do{P1M1|=SET_BIT0;P1M2|=SET_BIT0;}while(0)
#define P11_OpenDrain_Mode	do{P1M1|=SET_BIT1;P1M2|=SET_BIT1;}while(0)
#define P12_OpenDrain_Mode	do{P1M1|=SET_BIT2;P1M2|=SET_BIT2;}while(0)
#define P13_OpenDrain_Mode	do{P1M1|=SET_BIT3;P1M2|=SET_BIT3;}while(0)
#define P14_OpenDrain_Mode	do{P1M1|=SET_BIT4;P1M2|=SET_BIT4;}while(0)
#define P15_OpenDrain_Mode	do{P1M1|=SET_BIT5;P1M2|=SET_BIT5;}while(0)
#define P16_OpenDrain_Mode	do{P1M1|=SET_BIT6;P1M2|=SET_BIT6;}while(0)
#define P17_OpenDrain_Mode	do{P1M1|=SET_BIT7;P1M2|=SET_BIT7;}while(0)
#define P30_OpenDrain_Mode	do{P3M1|=SET_BIT0;P3M2|=SET_BIT0;}while(0)
//--------- Define all port as quasi mode ---------
#define Set_All_GPIO_Quasi_Mode	do{P0M1=0;P0M2=0;P1M1=0;P1M2=0;P3M1=0;P3M2=0;}while(0)

#define set_GPIO1	P12=1
#define clr_GPIO1	P12=0

/****************************************************************************
   Enable INT port 0~3
***************************************************************************/
#define Enable_INT_Port0	PICON &= 0xFB;
#define	Enable_INT_Port1	PICON |= 0x01;
#define	Enable_INT_Port2	PICON |= 0x02;
#define	Enable_INT_Port3	PICON |= 0x03;
/*****************************************************************************
 Enable each bit low level trig mode
*****************************************************************************/
#define	Enable_BIT7_LowLevel_Trig	do{PICON&=0x7F;PINEN|=0x80;PIPEN&=0x7F;}while(0)
#define	Enable_BIT6_LowLevel_Trig	do{PICON&=0x7F;PINEN|=0x40;PIPEN&=0xBF;}while(0)
#define	Enable_BIT5_LowLevel_Trig	do{PICON&=0xBF;PINEN|=0x20;PIPEN&=0xDF;}while(0)
#define	Enable_BIT4_LowLevel_Trig	do{PICON&=0xBF;PINEN|=0x10;PIPEN&=0xEF;}while(0)
#define	Enable_BIT3_LowLevel_Trig	do{PICON&=0xDF;PINEN|=0x08;PIPEN&=0xF7;}while(0)
#define	Enable_BIT2_LowLevel_Trig	do{PICON&=0xEF;PINEN|=0x04;PIPEN&=0xFB;}while(0)
#define	Enable_BIT1_LowLevel_Trig	do{PICON&=0xF7;PINEN|=0x02;PIPEN&=0xFD;}while(0)
#define	Enable_BIT0_LowLevel_Trig	do{PICON&=0xFD;PINEN|=0x01;PIPEN&=0xFE;}while(0)
/*****************************************************************************
 Enable each bit high level trig mode
*****************************************************************************/
#define	Enable_BIT7_HighLevel_Trig	do{PICON&=0x7F;PINEN&=0x7F;PIPEN|=0x80;}while(0)
#define	Enable_BIT6_HighLevel_Trig	do{PICON&=0x7F;PINEN&=0xBF;PIPEN|=0x40;}while(0)
#define	Enable_BIT5_HighLevel_Trig	do{PICON&=0xBF;PINEN&=0xDF;PIPEN|=0x20;}while(0)
#define	Enable_BIT4_HighLevel_Trig	do{PICON&=0xBF;PINEN&=0xEF;PIPEN|=0x10;}while(0)
#define	Enable_BIT3_HighLevel_Trig	do{PICON&=0xDF;PINEN&=0xF7;PIPEN|=0x08;}while(0)
#define	Enable_BIT2_HighLevel_Trig	do{PICON&=0xEF;PINEN&=0xFB;PIPEN|=0x04;}while(0)
#define	Enable_BIT1_HighLevel_Trig	do{PICON&=0xF7;PINEN&=0xFD;PIPEN|=0x02;}while(0)
#define	Enable_BIT0_HighLevel_Trig	do{PICON&=0xFD;PINEN&=0xFE;PIPEN|=0x01;}while(0)
/*****************************************************************************
 Enable each bit falling edge trig mode
*****************************************************************************/
#define	Enable_BIT7_FallEdge_Trig	do{PICON|=0x80;PINEN|=0x80;PIPEN&=0x7F;}while(0)
#define	Enable_BIT6_FallEdge_Trig	do{PICON|=0x80;PINEN|=0x40;PIPEN&=0xBF;}while(0)
#define	Enable_BIT5_FallEdge_Trig	do{PICON|=0x40;PINEN|=0x20;PIPEN&=0xDF;}while(0)
#define	Enable_BIT4_FallEdge_Trig	do{PICON|=0x40;PINEN|=0x10;PIPEN&=0xEF;}while(0)
#define	Enable_BIT3_FallEdge_Trig	do{PICON|=0x20;PINEN|=0x08;PIPEN&=0xF7;}while(0)
#define	Enable_BIT2_FallEdge_Trig	do{PICON|=0x10;PINEN|=0x04;PIPEN&=0xFB;}while(0)
#define	Enable_BIT1_FallEdge_Trig	do{PICON|=0x08;PINEN|=0x02;PIPEN&=0xFD;}while(0)
#define	Enable_BIT0_FallEdge_Trig	do{PICON|=0x04;PINEN|=0x01;PIPEN&=0xFE;}while(0)
/*****************************************************************************
 Enable each bit raising edge trig mode
*****************************************************************************/
#define	Enable_BIT7_RasingEdge_Trig	do{PICON|=0x80;PINEN&=0x7F;PIPEN|=0x80;}while(0)
#define	Enable_BIT6_RasingEdge_Trig	do{PICON|=0x80;PINEN&=0xBF;PIPEN|=0x40;}while(0)
#define	Enable_BIT5_RasingEdge_Trig	do{PICON|=0x40;PINEN&=0xDF;PIPEN|=0x20;}while(0)
#define	Enable_BIT4_RasingEdge_Trig	do{PICON|=0x40;PINEN&=0xEF;PIPEN|=0x10;}while(0)
#define	Enable_BIT3_RasingEdge_Trig	do{PICON|=0x20;PINEN&=0xF7;PIPEN|=0x08;}while(0)
#define	Enable_BIT2_RasingEdge_Trig	do{PICON|=0x10;PINEN&=0xFB;PIPEN|=0x04;}while(0)
#define	Enable_BIT1_RasingEdge_Trig	do{PICON|=0x08;PINEN&=0xFD;PIPEN|=0x02;}while(0)
#define	Enable_BIT0_RasingEdge_Trig	do{PICON|=0x04;PINEN&=0xFE;PIPEN|=0x01;}while(0)

/*****************************************************************************************
* For TIMER VALUE setting for TM0/TM1 and TM2/TM3
*****************************************************************************************/
#ifdef FOSC_16000		// if Fsys = 16MHz
	#define TIMER_DIV12_VALUE_10us		65536-13	// 13*12/16000000 = 10 uS,  	// Timer divider = 12 for TM0/TM1
	#define TIMER_DIV12_VALUE_100us		65536-130	// 130*12/16000000 = 10 uS,  	// Timer divider = 12
	#define TIMER_DIV12_VALUE_1ms		65536-1334	// 1334*12/16000000 = 1 mS, 	// Timer divider = 12
	#define TIMER_DIV12_VALUE_10ms		65536-13334	// 13334*12/16000000 = 10 mS 	// Timer divider = 12
	#define TIMER_DIV12_VALUE_40ms		65536-53336	// 53336*12/16000000 = 40 ms	// Timer divider = 12
	#define TIMER_DIV4_VALUE_10us		65536-40	// 40*4/16000000 = 10 uS,    	// Timer divider = 4 for TM2/TM3
	#define TIMER_DIV4_VALUE_100us		65536-400	// 400*4/16000000 = 100 us		// Timer divider = 4
	#define TIMER_DIV4_VALUE_200us		65536-800	// 800*4/16000000 = 200 us		// Timer divider = 4
	#define TIMER_DIV4_VALUE_500us		65536-2000	// 2000*4/16000000 = 500 us		// Timer divider = 4
    #define TIMER_DIV4_VALUE_1ms		65536-4000	// 4000*4/16000000 = 1 mS,   	// Timer divider = 4
	#define TIMER_DIV16_VALUE_10ms		65536-10000	// 10000*16/16000000 = 10 ms	// Timer divider = 16
	#define TIMER_DIV64_VALUE_30ms		65536-7500	// 7500*64/16000000 = 30 ms		// Timer divider = 64
	#define	TIMER_DIV128_VALUE_100ms	65536-12500	// 12500*128/16000000 = 100 ms	// Timer divider = 128
	#define	TIMER_DIV128_VALUE_200ms	65536-25000	// 25000*128/16000000 = 200 ms	// Timer divider = 128
	#define TIMER_DIV256_VALUE_500ms	65536-31250	// 31250*256/16000000 = 500 ms 	// Timer divider = 256
	#define	TIMER_DIV512_VALUE_1s		65536-31250	// 31250*512/16000000 = 1 s.  	// Timer Divider = 512
#endif

//-------------------- Timer0 function define --------------------
#define	TIMER1_MODE0_ENABLE		TMOD&=0x0F
#define	TIMER1_MODE1_ENABLE		do{TMOD&=0x0F;TMOD|=0x10;}while(0)
#define	TIMER1_MODE2_ENABLE		do{TMOD&=0x0F;TMOD|=0x20;}while(0)
#define	TIMER1_MODE3_ENABLE		do{TMOD&=0x0F;TMOD|=0x30;}while(0)
//-------------------- Timer1 function define --------------------
#define	TIMER0_MODE0_ENABLE		TMOD&=0xF0
#define	TIMER0_MODE1_ENABLE		do{TMOD&=0xF0;TMOD|=0x01;}while(0)
#define	TIMER0_MODE2_ENABLE		do{TMOD&=0xF0;TMOD|=0x02;}while(0)
#define	TIMER0_MODE3_ENABLE		do{TMOD&=0xF0;TMOD|=0x03;}while(0)
//-------------------- Timer2 function define --------------------
#define TIMER2_DIV_4			do{T2MOD|=0x10;T2MOD&=0x9F;}while(0)
#define TIMER2_DIV_16			do{T2MOD|=0x20;T2MOD&=0xAF;}while(0)
#define TIMER2_DIV_32			do{T2MOD|=0x30;T2MOD&=0xBF;}while(0)
#define TIMER2_DIV_64			do{T2MOD|=0x40;T2MOD&=0xCF;}while(0)
#define	TIMER2_DIV_128			do{T2MOD|=0x50;T2MOD&=0xDF;}while(0)
#define TIMER2_DIV_256			do{T2MOD|=0x60;T2MOD&=0xEF;}while(0)
#define TIMER2_DIV_512			T2MOD|=0x70
#define TIMER2_Auto_Reload_Delay_Mode	do{T2CON&=~SET_BIT0;T2MOD|=SET_BIT7;T2MOD|=SET_BIT3;}while(0)
#define TIMER2_Auto_Reload_Capture_Mode	do{T2CON&=~SET_BIT0;T2MOD|=SET_BIT7;}while(0)
#define	TIMER2_Compare_Capture_Mode		do{T2CON|=SET_BIT0;T2MOD&=~SET_BIT7;T2MOD|=SET_BIT2;}while(0)

#define TIMER2_CAP0_Capture_Mode		do{T2CON&=~SET_BIT0;T2MOD=0x89;}while(0)
#define TIMER2_CAP1_Capture_Mode		do{T2CON&=~SET_BIT0;T2MOD=0x8A;}while(0)
#define TIMER2_CAP2_Capture_Mode		do{T2CON&=~SET_BIT0;T2MOD=0x8B;}while(0)

//-------------------- Timer2 Capture define --------------------
//--- Falling Edge -----
#define IC0_P12_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC1_P11_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x01;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC2_P10_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x02;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC3_P00_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x03;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC3_P04_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x04;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC4_P01_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x05;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC5_P03_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x06;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC6_P05_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x07;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC7_P15_CAP0_FallingEdge_Capture	do{CAPCON1&=0xFC;CAPCON3&=0xF0;CAPCON3|=0x08;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)

#define IC0_P12_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC1_P11_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x10;CAPCON0|=SET_BIT5;CAPCON0|=SET_BIT5;}while(0)
#define	IC2_P10_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x20;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC3_P00_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x30;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC3_P04_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x40;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC4_P01_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x50;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC5_P03_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x60;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC6_P05_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x70;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC7_P15_CAP1_FallingEdge_Capture	do{CAPCON1&=0xF3;CAPCON3&=0x0F;CAPCON3|=0x80;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)

#define IC0_P12_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC1_P11_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x10;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC2_P10_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x20;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC3_P00_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x30;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC3_P04_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x40;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC4_P01_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x50;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC5_P03_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x60;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC6_P05_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x70;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC7_P15_CAP2_FallingEdge_Capture	do{CAPCON1&=0x0F;CAPCON4&=0xF0;CAPCON4|=0x80;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)

//----- Rising edge ----
#define IC0_P12_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC1_P11_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x01;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC2_P10_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x02;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC3_P00_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x03;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC3_P04_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x04;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC4_P01_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x05;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC5_P03_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x06;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC6_P05_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x07;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC7_P15_CAP0_RisingEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x01;CAPCON3&=0xF0;CAPCON3|=0x08;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)

#define IC0_P12_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC1_P11_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x10;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC2_P10_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x20;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC3_P00_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x30;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC3_P04_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x40;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC4_P01_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x50;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC5_P03_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x60;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC6_P05_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x70;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC7_P15_CAP1_RisingEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x04;CAPCON3&=0x0F;CAPCON3|=0x80;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;

#define IC0_P12_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC1_P11_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x01;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC2_P10_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x02;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC3_P00_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x03;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC3_P04_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x04;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC4_P01_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x05;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC5_P03_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x06;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC6_P05_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x07;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC7_P15_CAP3_RisingEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x10;CAPCON4&=0xF0;CAPCON4|=0x08;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)

//-----BOTH  edge ----
#define IC0_P12_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC1_P11_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x01;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC2_P10_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x02;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC3_P00_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x03;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC3_P04_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x04;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC4_P01_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x05;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC5_P03_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x06;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC6_P05_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x07;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)
#define	IC7_P15_CAP0_BothEdge_Capture	do{CAPCON1&=0xFC;CAPCON1|=0x02;CAPCON3&=0xF0;CAPCON3|=0x08;CAPCON0|=SET_BIT4;CAPCON2|=SET_BIT4;}while(0)

#define IC0_P12_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC1_P11_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x10;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC2_P10_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x20;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC3_P00_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x30;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC3_P04_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x40;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC4_P01_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x50;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC5_P03_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x60;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC6_P05_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x70;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)
#define	IC7_P15_CAP1_BothEdge_Capture	do{CAPCON1&=0xF3;CAPCON1|=0x08;CAPCON3&=0x0F;CAPCON3|=0x80;CAPCON0|=SET_BIT5;CAPCON2|=SET_BIT5;}while(0)

#define IC0_P12_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC1_P11_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x01;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC2_P10_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x02;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC3_P00_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x03;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC3_P04_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x04;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC4_P01_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x05;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC5_P03_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x06;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC6_P05_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x07;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)
#define	IC7_P15_CAP3_BothEdge_Capture	do{CAPCON1&=0x0F;CAPCON1|=0x20;CAPCON4&=0xF0;CAPCON4|=0x08;CAPCON0|=SET_BIT6;CAPCON2|=SET_BIT6;}while(0)

#define TIMER2_IC2_DISABLE				CAPCON0&=~SET_BIT6
#define TIMER2_IC1_DISABLE				CAPCON0&=~SET_BIT5
#define TIMER2_IC0_DISABLE				CAPCON0&=~SET_BIT4

/*****************************************************************************************
* For PWM setting
*****************************************************************************************/
//--------- PMW clock source select define ---------------------
#define	PWM_CLOCK_FSYS			CKCON &= 0xBF
#define	PWM_CLOCK_TIMER1		CKCON |= 0x40
//--------- PWM clock divide define ----------------------------
#define	PWM_CLOCK_DIV_1			(PWMCON1 &= 0xF8)
#define	PWM_CLOCK_DIV_2			do{PWMCON1|=0x01;PWMCON1&=0xF9;}while(0)
#define	PWM_CLOCK_DIV_4			do{PWMCON1|=0x02;PWMCON1&=0xFA;}while(0)
#define	PWM_CLOCK_DIV_8			do{PWMCON1|=0x03;PWMCON1&=0xFB;}while(0)
#define	PWM_CLOCK_DIV_16		do{PWMCON1|=0x04;PWMCON1&=0xFC;}while(0)
#define	PWM_CLOCK_DIV_32		do{PWMCON1|=0x05;PWMCON1&=0xFD;}while(0)
#define	PWM_CLOCK_DIV_64		do{PWMCON1|=0x06;PWMCON1&=0xFE;}while(0)
#define	PWM_CLOCK_DIV_128		(PWMCON1 |= 0x07)
//--------- PWM I/O select define ------------------------------
// P1.5 as PWM5 output enable
#define	PWM5_P15_OUTPUT_ENABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1|=0x20;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P0.3 as PWM5
#define	PWM5_P03_OUTPUT_ENABLE	PIOCON0|=0x20
// P0.1 as PWM4 output enable
#define	PWM4_P01_OUTPUT_ENABLE	PIOCON0|=0x10
// P0.4 as PWM3 output enable
#define	PWM3_P04_OUTPUT_ENABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1|=0x08;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P0.0 as PWM3
#define	PWM3_P00_OUTPUT_ENABLE	PIOCON0|=0x08
// P1.0 as PWM2 output enable
#define	PWM2_P05_OUTPUT_ENABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1|=0x04;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P1.0 as PWM2
#define	PWM2_P10_OUTPUT_ENABLE	PIOCON0|=0x04
// P1.4 as PWM1 output enable
#define	PWM1_P14_OUTPUT_ENABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1|=0x02;SET_TA;SFRS=0x00;EA=EA_SAV;}while(0)E
// P1.1 as PWM1
#define	PWM1_P11_OUTPUT_ENABLE	PIOCON0|=0x02
// P1.2 as PWM0 output enable
#define	PWM0_P12_OUTPUT_ENABLE	PIOCON0|=0x01
#define ALL_PWM_OUTPUT_ENABLE	do{PIOCON0=0xFF;PIOCON1=0xFF;}while(0)
// P1.5 as PWM5 output disable
#define	PWM5_P15_OUTPUT_DISABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1&=0xDF;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P0.3 as PWM5
#define	PWM5_P03_OUTPUT_DISABLE	PIOCON0&=0xDF
// P0.1 as PWM4 output disable
#define	PWM4_P01_OUTPUT_DISABLE	PIOCON0&=0xEF
// P0.4 as PWM3 output disable
#define	PWM3_P04_OUTPUT_DISABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1&=0xF7;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P0.0 as PWM3
#define	PWM3_P00_OUTPUT_DISABLE	PIOCON0&=0xF7
// P1.0 as PWM2 output disable
#define	PWM2_P05_OUTPUT_DISABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1&=0xFB;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P1.0 as PWM2
#define	PWM2_P10_OUTPUT_DISABLE	PIOCON0&=0xFB
// P1.4 as PWM1 output disable
#define	PWM1_P14_OUTPUT_DISABLE	do{SET_EA_TA;SFRS=0x01;PIOCON1&=0xFD;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
// P1.1 as PWM1
#define	PWM1_P11_OUTPUT_DISABLE	PIOCON0&=0xFD
// P1.2 as PWM0 output disable
#define	PWM0_P12_OUTPUT_DISABLE	PIOCON0&=0xFE
#define ALL_PWM_OUTPUT_DISABLE	do{PIOCON0=0x00;PIOCON1=0x00;}while(0)
//--------- PWM I/O Polarity Control ---------------------------
#define	PWM5_OUTPUT_INVERSE		PNP|=0x20
#define	PWM4_OUTPUT_INVERSE		PNP|=0x10
#define	PWM3_OUTPUT_INVERSE		PNP|=0x08
#define	PWM2_OUTPUT_INVERSE		PNP|=0x04
#define	PWM1_OUTPUT_INVERSE		PNP|=0x02
#define	PWM0_OUTPUT_INVERSE		PNP|=0x01
#define	PWM_OUTPUT_ALL_INVERSE	PNP=0xFF
#define	PWM5_OUTPUT_NORMAL		PNP&=0xDF
#define	PWM4_OUTPUT_NORMAL		PNP&=0xEF
#define	PWM3_OUTPUT_NORMAL		PNP&=0xF7
#define	PWM2_OUTPUT_NORMAL		PNP&=0xFB
#define	PWM1_OUTPUT_NORMAL		PNP&=0xFD
#define	PWM0_OUTPUT_NORMAL		PNP&=0xFE
#define	PWM_OUTPUT_ALL_NORMAL	PNP=0x00
/*------------------------ PWM type define -----------------------------------*/
#define	PWM_EDGE_TYPE			PWMCON1&=~SET_BIT4
#define	PWM_CENTER_TYPE			PWMCON1|=SET_BIT4
/*------------------------ PWM mode define -----------------------------------*/
#define	PWM_IMDEPENDENT_MODE	PWMCON1&=0x3F
#define	PWM_COMPLEMENTARY_MODE	do{PWMCON1|=0x40;PWMCON1&=0x7F;}while(0)
#define	PWM_SYNCHRONIZED_MODE	do{PWMCON1|=0x80;PWMCON1&=0xBF;}while(0)
#define PWM_GP_MODE_ENABLE		PWMCON1|=0x20
#define	PWM_GP_MODE_DISABLE		PWMCON1&=0xDF
/*------------------------ PMW interrupt setting -----------------------------*/
#define	PWM_FALLING_INT			do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xCF;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_RISING_INT			do{SET_EA_TA;SFRS=0x01;PWMINTC|=0x10;PWMCON0&=0xDF;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_CENTRAL_POINT_INT	do{SET_EA_TA;SFRS=0x01;PWMINTC|=0x20;PWMCON0&=0xEF;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_PERIOD_END_INT		do{SET_EA_TA;SFRS=0x01;PWMINTC|=0x30;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
/*----------------------- PWM interrupt pin select ---------------------------*/
#define	PWM_INT_PWM0	do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xF8;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_INT_PWM1	do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xF8;PWMINTC|=0x01;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_INT_PWM2	do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xF8;PWMINTC|=0x02;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_INT_PWM3	do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xF8;PWMINTC|=0x03;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_INT_PWM4	do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xF8;PWMINTC|=0x04;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
#define	PWM_INT_PWM5	do{SET_EA_TA;SFRS=0x01;PWMINTC&=0xF8;PWMINTC|=0x05;SET_TA;SFRS=0x00;EA=EA_SAVE;}while(0)
/*------------------------ PWM Dead time setting -----------------------------*/
#define PWM45_DEADTIME_ENABLE	do{SET_EA_TA;__asm__("orl _PDTEN,#0x04");EA=EA_SAVE;}while(0)
#define PWM34_DEADTIME_ENABLE	do{SET_EA_TA;__asm__("orl _PDTEN,#0x02");EA=EA_SAVE;}while(0)
#define PWM01_DEADTIME_ENABLE	do{SET_EA_TA;__asm__("orl _PDTEN,#0x01");EA=EA_SAVE;}while(0)

/*******************************************************************************
* For ADC INIT setting
*******************************************************************************/
// P17
#define Enable_ADC_AIN0		do{ADCCON0&=0xF0;P17_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT0;ADCCON1|=SET_BIT0;}while(0)
// P30
#define Enable_ADC_AIN1		do{ADCCON0&=0xF0;ADCCON0|=0x01;P30_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT1;ADCCON1|=SET_BIT0;}while(0)
// P07
#define Enable_ADC_AIN2		do{ADCCON0&=0xF0;ADCCON0|=0x02;P07_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT2;ADCCON1|=SET_BIT0;}while(0)
// P06
#define Enable_ADC_AIN3		do{ADCCON0&=0xF0;ADCCON0|=0x03;P06_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT3;ADCCON1|=SET_BIT0;}while(0)
// P05
#define Enable_ADC_AIN4		do{ADCCON0&=0xF0;ADCCON0|=0x04;P05_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT4;ADCCON1|=SET_BIT0;}while(0)
// P04
#define Enable_ADC_AIN5		do{ADCCON0&=0xF0;ADCCON0|=0x05;P04_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT5;ADCCON1|=SET_BIT0;}while(0)
// P03
#define Enable_ADC_AIN6		do{ADCCON0&=0xF0;ADCCON0|=0x06;P03_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT6;ADCCON1|=SET_BIT0;}while(0)
// P11
#define Enable_ADC_AIN7		do{ADCCON0&=0xF0;ADCCON0|=0x07;P11_Input_Mode;AINDIDS=0x00;AINDIDS|=SET_BIT7;ADCCON1|=SET_BIT0;}while(0)
// ref 1.22V
#define Enable_ADC_BandGap	do{ADCCON0|=SET_BIT3;ADCCON0&=0xF8;ADCCON1|=SET_BIT0;}while(0)
#define Disable_ADC			ADCCON1&=0xFE;

#define PWM0_FALLINGEDGE_TRIG_ADC	do{ADCCON0&=~SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1&=~SET_BIT3;ADCCON1&=~SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM2_FALLINGEDGE_TRIG_ADC	do{ADCCON0&=~SET_BIT5;ADCCON0|=SET_BIT4;ADCCON1&=~SET_BIT3;ADCCON1&=~SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM4_FALLINGEDGE_TRIG_ADC	do{ADCCON0|=SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1&=~SET_BIT3;ADCCON1&=~SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM0_RISINGEDGE_TRIG_ADC	do{ADCCON0&=~SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1&=~SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM2_RISINGEDGE_TRIG_ADC	do{ADCCON0&=~SET_BIT5;ADCCON0|=SET_BIT4;ADCCON1&=~SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM4_RISINGEDGE_TRIG_ADC	do{ADCCON0|=SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1&=~SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM0_CENTRAL_TRIG_ADC		do{ADCCON0&=~SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1|=SET_BIT3;ADCCON1&=~SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM2_CENTRAL_TRIG_ADC		do{ADCCON0&=~SET_BIT5;ADCCON0|=SET_BIT4;ADCCON1|=SET_BIT3;ADCCON1&=~SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM4_CENTRAL_TRIG_ADC		do{ADCCON0|=SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1|=SET_BIT3;ADCCON1&=~SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM0_END_TRIG_ADC			do{ADCCON0&=~SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1|=SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM2_END_TRIG_ADC			do{ADCCON0&=~SET_BIT5;ADCCON0|=SET_BIT4;ADCCON1|=SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;}while(0)
#define PWM4_END_TRIG_ADC			do{ADCCON0|=SET_BIT5;ADCCON0&=~SET_BIT4;ADCCON1|=SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;}while(0)

#define P04_FALLINGEDGE_TRIG_ADC	do{ADCCON0|=0x30;ADCCON1&=0xF3;ADCCON1|=SET_BIT1;ADCCON1&=~SET_BIT6;}while(0)
#define P13_FALLINGEDGE_TRIG_ADC	do{ADCCON0|=0x30;ADCCON1&=0xF3;ADCCON1|=SET_BIT1;ADCCON1|=SET_BIT6;}while(0)
#define P04_RISINGEDGE_TRIG_ADC		do{ADCCON0|=0x30;ADCCON1&=~SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;ADCCON1&=~SET_BIT6;}while(0)
#define P13_RISINGEDGE_TRIG_ADC		do{ADCCON0|=0x30;ADCCON1&=~SET_BIT3;ADCCON1|=SET_BIT2;ADCCON1|=SET_BIT1;ADCCON1|=SET_BIT6;}while(0)

/*******************************************************************************
* For SPI INIT setting
*******************************************************************************/
#define	SPICLK_DIV2				do{clr_SPR0;clr_SPR1;}while(0)
#define	SPICLK_DIV4				do{set_SPR0;clr_SPR1;}while(0)
#define	SPICLK_DIV8				do{clr_SPR0;set_SPR1;}while(0)
#define	SPICLK_DIV16			do{set_SPR0;set_SPR1;}while(0)
#define	Enable_SPI_Interrupt	do{set_ESPI;set_EA;}while(0)
#define	SS						P15

/**
 * Set of General Purpose/User Defiend Flags (or bits)
 */
#define set_gf0() (PCON |= SET_BIT2)
#define set_gf1() (PCON |= SET_BIT3)
#define set_gf2() (AUXR1 |= SET_BIT3)
#define set_f0() (F0 = 1) /* PSW | SET_BIT5 */
#define set_f1() (F1 = 1) /* PSW | SET_BIT1 */

#define get_gf0() (PCON & SET_BIT2)
#define get_gf1() (PCON & SET_BIT3)
#define get_gf2() (AUXR1 & SET_BIT3)
#define get_f0() F0 /* PSW & SET_BIT5 */
#define get_f1() F1 /* PSW & SET_BIT1 */

#define clr_gf0() (PCON &= ~SET_BIT2)
#define clr_gf1() (PCON &= ~SET_BIT3)
#define clr_gf2() (AUXR1 &= ~SET_BIT3)
#define clr_f0() (F0 = 0) /* PSW &= ~SET_BIT5, not for use by ISR */
#define clr_f1() (F1 = 0) /* PSW &= ~SET_BIT1, not for use by ISR */

#endif
