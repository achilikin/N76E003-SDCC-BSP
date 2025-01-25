/*------------------------------------------------------------------------------
  N76E003.C

  Some generic routines for SDCC compiler
  http://sdcc.sourceforge.net/

  Original header files for SFR bytes and bits,
  SFR Macros and Functions Defines are from OpenNuvoton project
  https://github.com/OpenNuvoton/N76E003-BSP
------------------------------------------------------------------------------*/
#include <N76E003.h>

__bit EA_SAVE;

/* Disable POR as recommended by N73E003 Series Errata Sheet */
unsigned char __sdcc_external_startup (void) __naked /* '__naked' is used to save on prologue */
{
    ta_enable();
    PORDIS = 0x5A;
    ta_enable();
    PORDIS = 0xA5;
   	__asm__("mov dpl,#0x00"); /* '__naked' is used, so generate epilogue manually */
    __asm__("ret");
}

/* a trick to get our code size using s_XINIT value generated by SDCC */
uint16_t sdcc_get_code_size(void) __naked
{
	__asm__("mov dpl,#s_XINIT");
	__asm__("mov dph,#(s_XINIT >> 8)");
	__asm__("ret");
}

#if defined FOSC_16600

uint8_t trim, trim0, trim1;

void set_fosc_16600(void)
{
	/* on power-on-reset store default HIRC trim value */
	/* and set Fsys to 16'600'000 Hz */
	if (PCON & SET_BIT4) {
		PCON &= ~SET_BIT4;
		trim = HIRC_TRIM;
		trim0 = RCTRIM0;
		trim1 = RCTRIM1;

		uint8_t hircmap0, hircmap1;
		uint16_t trim16;

		hircmap0 = trim0;
		hircmap1 = trim1;
		trim16 = ((hircmap0 << 1) + (hircmap1 & 0x01));
		trim16 = trim16 - trim;
		hircmap1 = trim16 & 0x01;
		hircmap0 = trim16 >> 1;
		ta_enable();
		RCTRIM0 = hircmap0;
		ta_enable();
		RCTRIM1 = hircmap1;
	}

	return;
}

#endif