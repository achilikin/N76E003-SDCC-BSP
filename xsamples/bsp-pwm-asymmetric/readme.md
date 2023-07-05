<!-- omit in toc -->
# PWM test application for Nuvoton N76E003 SDCC BSP

- [Overview](#overview)
- [Nuvoton N76E003 development board](#nuvoton-n76e003-development-board)
- [Supported commands:](#supported-commands)
	- [\> mode](#-mode)
	- [\> adc](#-adc)
	- [\> duty](#-duty)

# Overview

This example demonstrates how to generate two PWM signals by driving digital outputs using PWM interrupt handler.

These two PWM signals can be controlled independently (duty can be set per signal), in-phase (same duty for both signals) or anti-phase (signal 0 duty is in anti-phase for signal 1).

<img src="./img/diagram.svg"/>

# Nuvoton N76E003 development board
There are a few different Nuvoton N76E003 development boards available online. This one was used for the test app:

<img src="../../img/n76e003.jpg" width="400px"/><img src="../../img/schematics.png" width="440px"/>

```
             N76E0003 dev board pinout:
                G R C D V  + 3 T R G
                N S L A D  5 V X X N
                D T K T D  V 3 0 0 D
                | | | | |  | | | | |
                +----------------------+
        OUT0 ---|11 P1.4         VDD 9 |--- VDD
        OUT1 ---|12 P1.3         GND 7 |--- GND
        PWM0 ---|13 P1.2        P1.5 10|---
        PWM1 ---|14 P1.1        P1.6 8 |--- DAT [ICP]
        PWM2 ---|15 P1.0        P1.7 7 |---
             ---|16 P0.0        P3.0 5 |---
             ---|17 P0.1        P2.0 2 |--- RST [ICP]
   [ICP] CLK ---|18 P0.2        P0.7 3 |--- UART0 RX
        SYNC ---|19 P0.3        P0.6 2 |--- UART0 TX
        MARK ---|20 P0.4        P0.5 1 |--- ADC_CHANNEL
                +----------------------+

```
5 pins are used for oscilloscope only - ``PWM0``, ``PWM1``, ``PWM2``, ``SYNC``, ``MARK``, can be re-used as generic IO if needed.

ADC_CHANNEL (AIN4) reads output of 10K rotary potentiometer connected between VDD and GND. ADC value then used to control OUT1 and OUT2 duty in ``inphase`` and ``antiphase`` modes.


# Supported commands:
```
> help
VER: 2307.05 (4425 bytes)
CMD:
    reset
    adc [on|off]
    duty $0-1 [$u8]
    mode [independent|inphase|antiphase]
```

Used code and data:
```
   Name              Start    End  Size   Max Spare
   ---------------- ------ ------ ----- ----- -----------
   REG BANKS        0x0000 0x000F     2     4     2
   IDATA            0x0000 0x0053    84   256   172
   OVERLAYS                           2
   STACK            0x0054 0x00FF   172   248   172
   EXTERNAL RAM     0x0001 0x00c7   199   768   569 74.1% free
   ROM/EPROM/FLASH  0x0000 0x1148  4425 18432 14007 76.0% free
```

## > mode
Select operational mode for the signals:
* independent: signals are controlled independently using ``duty`` command
* inphase: duty for signal 0 applied to both signals (default on reset)
* antiphase: duty for signal 0 is in anti-phase for signal 1, calling ``duty 0 0`` will set OUT0 to 0% and OUT1 to 100%.

## > adc
If turned on (default on reset), then ADC value is used to set signals' duty in ``inphase`` and ``antiphase`` modes

## > duty
Sets duty counter for a channel, default on reset: 50%

