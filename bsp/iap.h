#ifndef N76E003_IAP_H
#define N76E003_IAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IAP_READ_CID    0x0B /**< company ID, return value 0xDA */
	#define IAP_CID 0xDA

#define IAP_READ_DID    0x0C /**< device ID, laddr 0 or 1, return value 0x5036 */
	#define IAP_DID 0x3650

#define IAP_READ_UID    0x04 /**< 96-bit unique code, laddr 0 to 11 */
	#define IAP_UID_SIZE 12

#define IAP_ERASE_APROM 0x22 /**< APROM page-erase */
#define IAP_ERASE_LDROM 0x62 /**< LDROM page-erase */

#define IAP_PROG_APROM  0x21 /**< APROM byte-program */
#define IAP_PROG_LDROM  0x61 /**< LDROM byte-program */

#define IAP_READ_APROM  0x00 /**< APROM byte-read */
#define IAP_READ_LDROM  0x40 /**< LDROM byte-read */

#define IAP_ERASE_CFG   0xE2 /**< erase all configuration registers */
#define IAP_PROG_CFG    0xE1 /**< program configuration registers 0 - 4 */
#define IAP_READ_CFG    0xC0 /**< configuration registers 0 - 4 */
	#define IAP_CFG_SIZE 5

#define iap_enable()  do{SET_EA_TA;__asm__("orl _CHPCON,#0x01");EA=EA_SAVE;}while(0)
#define iap_disable() do{SET_EA_TA;__asm__("anl _CHPCON,#0xFE");EA=EA_SAVE;}while(0)
#define iap_trigger() do{SET_EA_TA;__asm__("orl _IAPTRG,#0x01");EA=EA_SAVE;}while(0)
#define iap_clear_error()  do{SET_EA_TA;__asm__("anl _CHPCON,#0xBF");EA=EA_SAVE;}while(0)
#define iap_cfg_enable()   do{SET_EA_TA;__asm__("orl _IAPUEN,#0x04");EA=EA_SAVE;}while(0)
#define iap_ldrom_enable() do{SET_EA_TA;__asm__("orl _IAPUEN,#0x02");EA=EA_SAVE;}while(0)
#define iap_aprom_enable() do{SET_EA_TA;__asm__("orl _IAPUEN,#0x01");EA=EA_SAVE;}while(0)

#define iap_cfg_disable()   do{SET_EA_TA;__asm__("anl _IAPUEN,#0xFB");EA=EA_SAVE;}while(0)
#define iap_ldrom_disable() do{SET_EA_TA;__asm__("anl _IAPUEN,#0xFD");EA=EA_SAVE;}while(0)
#define iap_aprom_disable() do{SET_EA_TA;__asm__("anl _IAPUEN,#0xFE");EA=EA_SAVE;}while(0)

/**
 * cmd: IAP_READ_* for read
 * addr: data address or value index
 *
 * iap_enable() must be called before iap_read_cmd()
 * in case of error CHPCON bit 6 will be set
 */
uint8_t iap_read_cmd(uint8_t cmd, uint16_t addr);

/**
 * erase or program
 *
 * iap_enable() must be called before iap_write_cmd()
 * in case of error CHPCON bit 6 will be set
 */
void iap_write_cmd(uint8_t cmd, uint16_t addr, uint8_t data);

/** read company ID "0xDA" */
#define iap_read_cid() iap_read_cmd(IAP_READ_CID,0)

/** idx 0 for low byte (0x50) and 1 for high byte (0x36) */
#define iap_read_did(idx) iap_read_cmd(IAP_READ_DID,idx)

/** idx 0 to 11 for UID bytes */
#define iap_read_uid(idx) iap_read_cmd(IAP_READ_UID,idx)

/** idx 0 for low and 1 for high ADC bandgap bytes */
#define iap_read_adc_bandgap(idx) iap_read_cmd(IAP_READ_UID,idx+UID_SIZE)

/** addr is 16bit APROM byte to read */
#define iap_read_aprom(addr) iap_read_cmd(IAP_READ_APROM,addr)

#define iap_read_ldrom(addr) iap_read_cmd(IAP_READ_LDROM,addr)

/** idx for config bytes 0 - 4 */
#define iap_read_cfg(idx) iap_read_cmd(IAP_READ_CFG,idx)

#define iap_erase_aprom(addr) iap_write_cmd(IAP_ERASE_APROM,addr,0xFF)
#define iap_erase_ldrom(addr) iap_write_cmd(IAP_ERASE_LDROM,addr,0xFF)
#define iap_erase_cfg() iap_write_cmd(IAP_ERASE_CFG,0,0xFF)

#define iap_prog_aprom(addr,data) iap_write_cmd(IAP_PROG_APROM,addr,data)
#define iap_prog_ldrom(addr,data) iap_write_cmd(IAP_PROG_LDROM,addr,data)
#define iap_prog_cfg(idx,data) iap_write_cmd(IAP_PROG_CFG,idx,data)

#ifdef __cplusplus
}
#endif

#endif
