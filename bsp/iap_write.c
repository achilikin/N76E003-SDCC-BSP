#include <N76E003.h>

#include "iap.h"

void iap_write_cmd(uint8_t cmd, uint16_t addr, uint8_t data)
{
	IAPCN = cmd;
	IAPAH = HIBYTE(addr);
	IAPAL = LOBYTE(addr);
	IAPFD = data;
	iap_trigger();
}
