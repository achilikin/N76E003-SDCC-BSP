/**
 * dump buffer to serial port, usefull for debugging or memory dumping
 */
#ifndef MEM_DUMP_H
#define MEM_DUMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XBUF_SIZE 16
extern __xdata uint8_t xbuf[XBUF_SIZE];

/** print dump header */
void dump_header(void);

/**
 * dump current buffer on screen
 * @param addr: address to print, valid max address is x4800 (18k)
 * @param len: number of bytes in the buffer to print
 */
void dump_xbuf(uint16_t addr, uint8_t len);

/** copy data to buffer for dump */
void dump_set_bufc(__code const uint8_t *addr, uint8_t len);
void dump_set_bufi(__idata const uint8_t *addr, uint8_t len);

/**
 * read any SFR register
 * implemented as a switch with 128 cases, so should be used only
 * if enough of code memory is available
 *
 * @param addr SFR address, for example 0x80 for P0
 * if needed page 1 should be selected before calling sfr_read()
  */
uint8_t sfr_read(uint8_t addr);

#ifdef __cplusplus
}
#endif
#endif
