# N76E003-SDCC
[OpenNuvoton N76E003-BSP](https://github.com/OpenNuvoton/N76E003-BSP) adopted for
Small Device C Compiler [(SDCC)](http://sdcc.sourceforge.net/)
with some defines to make [Visual Studio Code](https://code.visualstudio.com/) happy

BSP itself compressed into a couple of bsp/N76E003.h and bsp/N76E003.c files with multiple auxiliary APIs/drivers located in the ``bsp`` and ``lib`` folders.

BSP requires SDCC version 4.2 and up (tested with SDCC versions 4.2.0 and 4.5.0).

Repository structure:
```
├── .vscode : some Visual Code settings and common tasks
│   └── tasks.json: defines the following tasks:
│       ├── build: build in the current folder
│       ├── clean: make clean in the current folder
│       ├── re-build: make clean; make; in the current folder
│       ├── erase device: erase all memory types and configuration bytes
│       └── flash device: flash the image from the current folder
├── bsp : common system files
│   ├── N76E003.c/h: main definitions for N76E003
│   ├── adc.c/h: ADC APIs
│   ├── event.c/h: simple ring buffer for generating events from ISRs
│   ├── i2c.c/h: I2C bus APIs
│   ├── iap*.c/h: In Application Programming routines to read/write MCU flash memory
│   ├── irq.c/h: interrupts handling APIs
│   ├── key.c/h: simple driver for keys (push buttons) connected to pull-up pins
│   ├── key.svg: diagram of keys handling and events generation
│   ├── pinterrupt.c/h: pin interrupt handling APIs
│   ├── pwm.c/h: PWM handling APIs
│   ├── terminal.c/h: serial communication APIs enough to support simple CLI with one line history
│   ├── tick.c/h: wake-up timer (WKT) interrupt to provide milliseconds tick events
│   ├── uart.c/h: UART 0/1 APIs.
│   └── vdd.c: ADC bandgap for calculating Vdd value
├── lib : library of common drivers
│   ├── bv4618.c/h: BV4618 LCD controller driver
│   ├── dht.c/h: DHT/AM2302 relative humidity/temperature driver
│   ├── ds3231.c/h: DS3231 Real Time Clock driver
│   ├── dump.c/h: simple pretty printer for memory dumps
│   ├── ht1621.c/h: Holtek HT1621 RAM Mapping 32x4 LCD Controller driver
│   ├── i2c_mem.c/h: I2C EEPROM 24C* driver
│   ├── lcd_lpwm.c/h: driver for LCD used to XY-LPWM and clone boards
│   ├── pcf8574.c/h: I2C I/O Extender PCF8574 driver
│   ├── pwm_range.c/h: helper functions to specify PWM frequency ranges
│   └── srfs.c: read any SFR register by its address
├── pys : python scripts
│   └── size-mcs51.py: .mem file parser for mcs51 target generated by sdcc linker
├── xsamples
│   ├── bsp-pwm: testing PWM capabilities of N76E003
│   ├── bsp-pwm-asymmetric: PWM signals generated by PWM interrupt handler
│   ├── bsp-spi: test different SPI configuration options
│   ├── bsp-template: template project for the BSP
│   ├── bsp-test: testing environment for bsp/lib
│   ├── hpdl-1414: HPDL-1414 Alphanumeric display example
│   ├── pwm-low-power: PWM to drive LED depending on Li-ion battery voltage,
│   │                  power-down mode example
│   ├── xy-lpwm-fw: alternative firmware for XY-LPWM and clones
│   └── xy-lpwm-lcd: XY-LPWM LCD screen controlled by commands over serial port
└── readme.md: this file
```

[bsp-pwm](./xsamples/bsp-pwm/readme.md) provides examples of PWM configurations and PWM interrupt processing, including ``phased`` PWM implemented using SW interrupt processing.

[bsp-pwm-asymmetric](./xsamples/bsp-pwm-asymmetric/readme.md) provides examples of PWM signals generated by PWM interrupt processing, including ``inphase`` and ``antiphase`` signals.

[bsp-spi](./xsamples/bsp-spi/readme.md) test some of SPI configuration options and use of ``SPSR::TXBUF`` flag.

[bsp-template](./xsamples/bsp-template/readme.md) provides skeleton example for the BSP.

[bsp-test](./xsamples/bsp-test/readme.md) provides examples of connecting different sensors/devices to N76E003 development board:

* DHT22 (AM2302) humidity/temperature sensor
* RTC [DS3231](https://www.maximintegrated.com/en/products/analog/real-time-clocks/DS3231.html) with I2C EEPROM Atmel [24C32](https://ww1.microchip.com/downloads/en/DeviceDoc/doc0336.pdf) on board
* 20x4 LCD with [BV4618](http://www.byvac.com/index.php/BV4618) as I2C controller
* 16x4 LCD with [PCF8574](https://www.ti.com/lit/ds/symlink/pcf8574.pdf) as I2C controller
* PS/2 Mini Keyboard UK Layout
* [HT16K33](https://www.holtek.com/documents/10179/116711/HT16K33v120.pdf) based quad alphanumeric display

[hpdl-1414](./xsamples/hpdl-1414/readme.md) HPDL-1414 Alphanumeric display controlled using MCP23017 I2C 16-Bit I/O Expander
[xy-lpwm-fw](./xsamples/xy-lpwm-fw/readme.md) and [xy-lpwm-lcd](./xsamples/xy-lpwm-lcd/readme.md) examples use popular XY-LPWM single channel PWM generator as a development board for N76E003.

XY-LPWM3 with 3 PWM channels would be even better dev board as it provides 8 keys instead or 4 and 3 PWM outputs instead of 1, example with XY-LPWM3 TDB.

# Events driven style
BSP implements a simple Even Driven Architecture (EDA) where events are generated by interrupts procedures and then processed in the main loop. By default two interrupts are handled and generates events: UART and Timer:
``` C
...
/**
 * generate EVT_TIMER every 250 msec, 4 times per second
 * set to 253 instead of 250 in case if 16.6MHz is used as Fsys
 * */
tick_init(250);
EA = 1; /* enable all interrupts to start tick timer */

...
/* events processing loop */
while (1) {
	evt.evt = event_get();

	if (evt.type) {
		if (evt.type == EVT_UART_RX) {
			cli_interact(evt.data);
			continue;
		}
		if (evt.type == EVT_TICK) {
			tick++;
			/**
			 * we have 4 tick events per second, so
			 * count  to 4 before calling our timer handler
			 * */
			if (tick >= 4) {
				timer();
				tick = 0;
			}
			continue;
		}
	}
}
```
See ``event.h`` and ``event.c`` for more details.


# Prerequisites
To compile any of the examples make sure that you have ``make`` (make from [``avr-gcc``](https://blog.zakkemble.net/avr-gcc-builds/) works just fine), [``sdcc``](http://sdcc.sourceforge.net/) and Nuvoton's ICP [``nulink``](https://github.com/OpenNuvoton/Nuvoton_Tools) in the path. If not, modify corresponding defines in the makefiles.

You also will need Python 3.x installed for ``size-mcs51.py`` to work.

This script parses .mem file generated by SDCC linker and list information about memory sections:
```
> size-mcs51.py main.mem
   Name              Start    End  Size   Max Spare
   ---------------- ------ ------ ----- ----- -----------
   REG BANKS        0x0000 0x000F     2     4     2
   IDATA            0x0000 0x008B   140   256   116
   OVERLAYS                           3
   STACK            0x008C 0x00FF   116   248   116
   EXTERNAL RAM     0x0001 0x00e6   230   768   538 70.1% free
   ROM/EPROM/FLASH  0x0000 0x36b7 14008 18432  4424 24.0% free
```

Visual Studio Code IntelliSence [configuration](.vscode/c_cpp_properties.json) expects ``SDCCPATH`` environment variable to point to SDCC installation folder (needed to access SDCC header files).

# Nuvoton N76E003 development boards
There are a few different Nuvoton N76E003 development boards available online.

## USB powered board
This one was used for the BSP examples:

<img src="./img/n76e003.jpg" width="400px"/>

<img src="./img/schematics-big-board.svg"/>

Minimal configuration uses programmer pins and UART (38400 baud rate):
```
                  G R C D V    + 3 T R G
                  N S L A D    5 V X X N
                  D T K T D    V 3 0 0 D
                  | | | | |    | | | | |
                 +----------------------+
              ---|11 P1.4         VDD 9 |--- VDD
              ---|12 P1.3         GND 7 |--- GND
              ---|13 P1.2        P1.5 10|---
              ---|14 P1.1        P1.6 8 |--- DAT [ICP]
              ---|15 P1.0        P1.7 7 |---
              ---|16 P0.0        P3.0 5 |---
              ---|17 P0.1        P2.0 2 |--- RST [ICP]
    [ICP] CLK ---|18 P0.2        P0.7 3 |--- UART0 RX
              ---|19 P0.3        P0.6 2 |--- UART0 TX
              ---|20 P0.4        P0.5 1 |---
                 +----------------------+
```
leaving 13 IO pins available.

## Small board
This one was used for the pwm-low-power example:

<img src="./img/n76e003-small.jpg" width="400px"/>

<img src="./img/schematics-small-board.svg"/>

```
                         G V R T
                         N D X X
                         D D 0 0
                         | | | |
                 +---------------------+
              ---|11 P1.4       P1.5 11|--- (BLINK)
              ---|12 P1.3        VDD 10|--- VDD
              ---|13 P1.2        VIN 9 |--- VIN
              ---|14 P1.1        GND 8 |--- GND
              ---|15 P1.0       P1.7 7 |---
              ---|16 P0.0       P3.0 5 |---
              ---|17 P0.1       P2.0 2 |--- RST [ICP]
    [ICP] CLK ---|18 P0.2       P0.7 3 |--- UART0 RX
              ---|19 P0.3       P0.6 2 |--- UART0 TX
              ---|20 P0.4       P0.5 1 |---
                 +---------------------+
                        | | | | |
                        G R C D V
                        N S L A D
                        D T K T D
```
Note that this board does not have P1.6 (DAT [ICP]) exposed on the side pins.

Pin P1.5 has on-board ``blink`` LED connected.

## XY-LPWM board and clones

<img src="./img/xy-lpwm-schematics.svg"/>

7133-1 voltage regulator (24V to 3.3V):

<img src="./img/jz-lpwm.jpg" width="400px"/>

M5333B voltage regulator (35V to 3.3V):

<img src="./img/vhm-800.jpg" width="400px"/>

Three channel board (not tested used for BSP):

<img src="./img/xy-lpwm3-schematics.svg"/>

<img src="./img/xy-lpwm3.jpg" width="400px"/>


# Reference
Nuvoton N76E003 official site https://www.nuvoton.com/products/microcontrollers/8bit-8051-mcus/low-pin-count-8051-series/n76e003/

NuMicro software development tools https://github.com/OpenNuvoton/Nuvoton_Tools

OpenNuvoton NuMicro 8051 Family BSP / Sample Code https://github.com/OpenNuvoton/NuMicro-8051-Family

OpenNuvoton N76E003 BSP v2.0 https://github.com/OpenNuvoton/N76E003_BSP

Small Device C Compiler http://sdcc.sourceforge.net/
