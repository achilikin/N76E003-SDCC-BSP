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

/** dump current buffer on sreen */
void dump_xbuf(uint16_t addr, uint8_t len);

/** copy data to buffer for dump */
void dump_set_bufc(__code const uint8_t *addr, uint8_t len);
void dump_set_bufi(__idata const uint8_t *addr, uint8_t len);

#ifdef __cplusplus
}
#endif
#endif
