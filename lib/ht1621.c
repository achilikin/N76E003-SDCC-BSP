/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for Holtek HT1621 RAM Mapping 32x4 LCD Controller
  For the latest datasheet see https://www.holtek.com/productdetail/-/vg/ht1621
*/
#include <N76E003.h>

#include "ht1621.h"

#define HT1621_WRCMD  0x00
#define HT1621_WRDATA 0x40

/* select and write start bit */
#define ht1621_select() \
	ht1621_set_cs(0); \
	ht1621_set_wr(0); \
	ht1621_set_data(1); \
	ht1621_set_wr(1)

/* unselect and restore data to 1 */
#define ht1621_unselect() ht1621_set_cs(1); ht1621_set_data(1)

/** generic write of 'len' MSB of the 'data' */
static void ht1621_write_msb(uint8_t data, uint8_t len)
{
	while (len) {
		ht1621_set_wr(0);
		ht1621_set_data(data & 0x80);
		ht1621_set_wr(1);
		data <<= 1;
		len--;
	}
}

/** generic write of 'len' LSB of the 'data' */
static void ht1621_write_lsb(uint8_t data, uint8_t len)
{
	while (len) {
		ht1621_set_wr(0);
		ht1621_set_data(data & 0x01);
		ht1621_set_wr(1);
		data >>= 1;
		len--;
	}
}

/**
 * Write command mode DATA pin sequence:
 *   1 0 0 C7 C6 C5 C4 C3 C2 C1 C0 X
 */
void ht1621_write_cmd(uint8_t cmd)
{
	ht1621_select();
	/* command is pushed MSB first */
	ht1621_write_msb(HT1621_WRCMD, 2);
	ht1621_write_msb(cmd, 8);
	ht1621_write_msb(0, 1);
	ht1621_unselect();
}

void ht1621_init(uint8_t bias_com_cfg)
{
	HT_CS_PIN_INIT();
	HT_WR_PIN_INIT();
	HT_DATA_PIN_INIT();

	ht1621_set_cs(1);
	ht1621_set_wr(1);
	ht1621_set_data(1);
	ht1621_write_cmd(bias_com_cfg);
	ht1621_write_cmd(HT1621_RC_256K);
	ht1621_write_cmd(HT1621_SYS_DIS);
	ht1621_write_cmd(HT1621_WDT_DIS);
	ht1621_write_cmd(HT1621_TONE_OFF);
	ht1621_write_cmd(HT1621_SYS_EN);
	ht1621_write_cmd(HT1621_LCD_ON);
}

/**
 * Write single segment data
 * @param seg 4bit segment index
 * @param data to be written to this segment (LSB first)
 * DATA pin write sequence:
 *   Mode  | Seg address (SA)  | Data at [SA]
 *   1 0 1 | A5 A4 A3 A2 A1 A0 | D0 D1 D2 D3
 */
void ht1621_write_seg(uint8_t seg, uint8_t data)
{
	uint8_t addr = seg & 0x3F;
	addr |= HT1621_WRDATA;
	ht1621_select();
	/* address is shifted MSB first */
	ht1621_write_msb(addr, 8);
	/* data is shifted LSB first */
	ht1621_write_lsb(data, 4);
	ht1621_unselect();
}

/**
 * Write successive segments data mode DATA pin sequence:
 *   Mode  | Seg address (SA)  | Data at [SA]| Data at [SA+1]
 *   1 0 1 | A5 A4 A3 A2 A1 A0 | D0 D1 D2 D3 | D0 D1 D2 D3
 */
void ht1621_write_data(uint8_t seg, uint8_t data)
{
	uint8_t addr = seg & 0x3F;
	addr |= HT1621_WRDATA;
	ht1621_select();
	/* address is shifted MSB first */
	ht1621_write_msb(addr, 8);
	/* data is shifted LSB first */
	ht1621_write_lsb(data, 8);
	ht1621_unselect();
}
