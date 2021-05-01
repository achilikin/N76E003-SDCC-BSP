/*
  The MIT License (MIT)

  Nuvoton N76E003 driver for MCP23017 I2C 16-Bit I/O Expander
  For the latest datasheet see https://www.microchip.com/wwwproducts/en/MCP23017
*/
#ifndef N76E003_MCP23017_H
#define N76E003_MCP23017_H

#include <N76E003.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MCP23017 I2C address */
#ifndef I2C_MCP23017
#define I2C_MCP23017 ((uint8_t)(0x20 << 1))
#endif

enum MCP_BANK0 {
	MCPB0_IODIRA,
	MCPB0_IODIRB,
	MCPB0_IPOLA,
	MCPB0_IPOLB,
	MCPB0_GPINTENA,
	MCPB0_GPINTENB,
	MCPB0_DEFVALA,
	MCPB0_DEFVALB,
	MCPB0_INTCONA,
	MCPB0_INTCONB,
	MCPB0_IOCON,
	MCPB0_IOCONB,
	MCPB0_GPPUA,
	MCPB0_GPPUB,
	MCPB0_INTFA,
	MCPB0_INTFB,
	MCPB0_INTCAPAA,
	MCPB0_INTCAPAB,
	MCPB0_GPIOA,
	MCPB0_GPIOB,
	MCPB0_OLATA,
	MCPB0_OLATB,
};

#ifdef __cplusplus
}
#endif
#endif