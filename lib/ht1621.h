/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for Holtek HT1621 RAM Mapping 32x4 LCD Controller
  For the latest datasheet see https://www.holtek.com/productdetail/-/vg/ht1621
*/
#ifndef N76E003_HT1621_H
#define N76E003_HT1621_H

#include <N76E003.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HT_CS_PIN
#define HT_CS_PIN P10
#define HT_CS_PIN_INIT() P10_PushPull_Mode
#endif
#ifndef HT_WR_PIN
#define HT_WR_PIN P00
#define HT_WR_PIN_INIT() P00_PushPull_Mode
#endif
#ifndef HT_DATA_PIN
#define HT_DATA_PIN P01
#define HT_DATA_PIN_INIT() P01_PushPull_Mode
#endif

/* HT1621 control defines */
/* works without nops(), but better have them just in case */
#define ht1621_set_cs(val) do{HT_CS_PIN=val;nop();nop();}while(0)
#define ht1621_set_wr(val) do{HT_WR_PIN=val;nop();nop();}while(0)
#define ht1621_set_data(val) do{HT_DATA_PIN=val;nop();nop();}while(0)

/* HT1621 commands */
#define HT1621_SYS_DIS   0x00	/** Turn off both system oscillator and LCD bias generator (POR) */
#define HT1621_SYS_EN    0x01	/** Turn on system oscillator */
#define HT1621_LCD_OFF   0x02	/** Turn off LCD bias generator (POR) */
#define HT1621_LCD_ON    0x03	/** Turn on LCD bias generator */
#define HT1621_TIMER_DIS 0x04	/** Disable time base output */
#define HT1621_WDT_DIS   0x05	/** Disable WDT time-out flag output */
#define HT1621_TIMER_EN  0x06	/** Enable time base output */
#define HT1621_WDT_EN    0x07	/** Enable WDT time-out flag output */
#define HT1621_TONE_OFF  0x08	/** Turn off tone outputs (POR) */
#define HT1621_TONE_ON   0x09	/** Turn on tone outputs */
#define HT1621_CLR_TIMER 0x0C	/** Clear the contents of time base generator */
#define HT1621_CLR_WDT   0x0E	/** Clear the contents of WDT stage */
#define HT1621_XTAL_32K  0x14	/** System clock source, crystal oscillator */
#define HT1621_RC_256K   0x18	/** System clock source, on - chip RC oscillator (POR) */
#define HT1621_EXT_256K  0x1C	/** System clock source, external clock source */
#define HT1621_BIAS_2    0x20	/** LCD 1/2 bias option, must be combined with _COM* */
#define HT1621_BIAS_3    0x21	/** LCD 1/3 bias option, must be combined with _COM* */
#define HT1621_COM_2     0x00	/** 2 commons option */
#define HT1621_COM_3     0x04	/** 3 commons option */
#define HT1621_COM_4     0x08	/** 4 commons option */
#define HT1621_TONE_4K   0x40	/** Tone frequency, 4kHz */
#define HT1621_TONE_2K   0x06	/** Tone frequency, 2kHz */
#define HT1621_IRQ_DIS   0x80	/** Disable IRQ output (POR) */
#define HT1621_IRQ_EN    0x88	/** Enable IRQ output */
#define HT1621_F1        0xA0	/** Time base / WDT clock output : 1Hz */
								/** The WDT time - out flag after : 4s */
#define HT1621_F2        0xA1	/** Time base / WDT clock output : 2Hz */
								/** The WDT time - out flag after : 2s */
#define HT1621_F4		 0xA2	/** Time base / WDT clock output : 4Hz */
								/** The WDT time - out flag after : 1s */
#define HT1621_F8		 0xA3	/** Time base / WDT clock output : 8Hz */
								/** The WDT time - out flag after : 1 / 2s */
#define HT1621_F16		 0xA4	/** Time base / WDT clock output : 16Hz */
								/** The WDT time - out flag after : 1 / 4s */
#define HT1621_F32		 0xA5	/** Time base / WDT clock output : 32Hz */
								/** The WDT time - out flag after : 1 / 8s */
#define HT1621_F64		 0xA6	/** Time base / WDT clock output : 64Hz */
								/** The WDT time - out flag after : 1 / 16s */
#define HT1621_F128		 0xA7	/** Time base / WDT clock output : 128Hz */
								/** The WDT time - out flag after : 1 / 32s (POR) */
#define HT1621_TEST		 0xE0	/** Test mode, do not use */
#define HT1621_NORMAL	 0xE3	/** Normal mode (POR) */

/**
 * screen initialization
 * bias_com_cfg, for example, HT1621_BIAS_2 | HT1621_COM_4
 */
void ht1621_init(uint8_t bias_com_cfg);

/** write HT1621 command */
void ht1621_write_cmd(uint8_t cmd);

/** address of the segment to write 4 LSB of data */
void ht1621_write_seg(uint8_t seg, uint8_t data);

/** address of the starting segment and 8 bits of data */
void ht1621_write_data(uint8_t address, uint8_t data);

#ifdef __cplusplus
}
#endif
#endif
