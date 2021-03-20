#include <N76E003.h>

#include "iap.h"

uint8_t iap_read_cmd(uint8_t cmd, uint16_t addr)
{
	uint8_t data;

	IAPCN = cmd;
	IAPAH = HIBYTE(addr);
	IAPAL = LOBYTE(addr);
	iap_trigger();
	data = IAPFD;
	return data;
}
