<!-- omit in toc -->
# Application template for Nuvoton N76E003 SDCC BSP

- [Nuvoton N76E003 development board](#nuvoton-n76e003-development-board)
- [Supported commands:](#supported-commands)
- [Used code and data](#used-code-and-data)

# Nuvoton N76E003 development board
There are a few different Nuvoton N76E003 development boards available online. This one was used for the test app:

<img src="../../img/n76e003.jpg" width="400px"/><img src="../../img/schematics.png" width="440px"/>

```
             N76E0003 dev board pinout:
                G R C D V  + 3 T R G
                N S L A D  5 V X X N
                D T K T D  V 3 0 0 D
                | | | | |  | | | | |
                +------------------+
             ---| P1.4         VDD |--- VDD
             ---| P1.3         GND |--- GND
             ---| P1.2        P1.5 |---
             ---| P1.1        P1.6 |--- ICPDA [SDA]
             ---| P1.0        P1.7 |---
             ---| P0.0        P3.0 |---
             ---| P0.1        P2.0 |--- RST
[SCL] ICPCLK ---| P0.2        P0.7 |--- UART0 RX
             ---| P0.3        P0.6 |--- UART0 TX
             ---| P0.4        P0.5 |---
                +------------------+
```

# Supported commands:
```
> help
VER: 2103.28 (2961 bytes)
CMD:
    reset
```
# Used code and data
```
   Name              Start    End  Size   Max Spare
   ---------------- ------ ------ ----- ----- -----------
   REG BANKS        0x0000 0x000F     2     4     2
   IDATA            0x0000 0x0052    83   256   173
   OVERLAYS                           2
   STACK            0x0053 0x00FF   173   248   173
   EXTERNAL RAM     0x0001 0x00c7   199   768   569 74.1% free
   ROM/EPROM/FLASH  0x0000 0x0b90  2961 18432 15471 83.9% free
```

