<!-- omit in toc -->
# Switch Nuvoton N76E003 to power down mode on timer

- [Nuvoton N76E003 development board](#nuvoton-n76e003-development-board)
- [Supported commands:](#supported-commands)
	- [reset](#reset)
	- [vdd](#vdd)
- [Used code and data](#used-code-and-data)
- [State machine](#state-machine)
- [Power states](#power-states)

# Nuvoton N76E003 development board
In this example PWM is used to drive LED depending on Li-ion battery voltage.

When battery is low, or if LEN timer is OFF, Nuvoton switches to power-down mode.

Small version of Nuvoton N76E003 development board used:

<img src="../../img/n76e003-small.jpg" width="400px"/><img src="../../img/schematics-small-board.svg" width="440px"/>

```
                        G V R T
                        N D X X
                        D D 0 0
                        | | | |
                +---------------------+
 STATE.OUT.1 ---|11 P1.4       P1.5 11|--- (BLINK)
 STATE.OUT.0 ---|12 P1.3        VDD 10|--- VDD
        PWM0 ---|13 P1.2        VIN 9 |--- VDD (VIN)
         CLO ---|14 P1.1        GND 8 |--- GND
             ---|15 P1.0       P1.7 7 |---
             ---|16 P0.0       P3.0 5 |--- SLEEP_TIMER
   SLEEP.OUT ---|17 P0.1       P2.0 2 |--- RST [ICP]
   [ICP] CLK ---|18 P0.2       P0.7 3 |--- UART0 RX
             ---|19 P0.3       P0.6 2 |--- UART0 TX
        MARK ---|20 P0.4       P0.5 1 |---
                +---------------------+
                       | | | | |
                       G R C D V
                       N S L A D
                       D T K T D
```
Board was modified to save power - AMS1117 volatge regulator remoed and Power led disconnected.

# Supported commands:
```
> help
VER: 2310.06 (5886 bytes)
BGP: 0672 1237 mV
Vdd: 3604 mV
CMD:
    vdd
    reset
```
## reset
Trigger soft reset

## vdd
Measure and print current Vdd.
```
> vdd
Vdd: 3604 mV, PWM: 85 %
```

Note that UART is active only when LED is ON.

# Used code and data
```
   Name              Start    End  Size   Max Spare
   ---------------- ------ ------ ----- ----- -----------
   REG BANKS        0x0000 0x000F     2     4     2
   IDATA            0x0000 0x0055    86   256   170
   OVERLAYS                           2
   STACK            0x0056 0x00FF   170   248   170
   EXTERNAL RAM     0x0001 0x00c7   199   768   569 74.1% free
   ROM/EPROM/FLASH  0x0000 0x16fd  5886 18432 12546 68.1% free
```

# State machine
<img src="./img/state-machine.svg"/>

Three states:

**STATE_LED_ON**

SLEEP_TIMER_PIN = 0, Vdd >= 3.0V.

Fsys 16MHz, WKT timer fires every 1 msec.

PWM0 used to drive LED according to lithium-ion battery voltage, PWM duty changes between 70% (Vdd 4.2V) and 100% (Vdd 3.0V).

**STATE_LED_OFF**

SLEEP_TIMER_PIN = 1, Vdd >= 3.0V.

Fsys 10MHz, WKT timer fires every 1 minute to check Vdd.

**STATE_LED_OFF**

SLEEP_TIMER_PIN = ?, Vdd < 3.0V.

Fsys 10MHz, WKT timer fires every 1 minute to check Vdd.

# Power states
Some tricks to save power in power-down mode:
* Disable BOD ``clr_BODEN; /* disable BOD circuit to save power in power down mode */``
* Disable IAP ``iap_disable();`` if you have it enabled
* Switch to 10KHz low speed oscillator ``fsys_set_clock(FSYS_10KHZ);``
* Set maximal WKT timer interval ``wkt_tick_set_mode(WKT_TICK_MINUTE);``

Combining all the above will help to achieve ~6 uA:

<img src="./img/power-down.png" width="600" />

with ~300uA every minute to check Vdd

With LED and timer not attached in Power ON mode this board will consume ~ 6mA:

<img src="./img/power-on.png" width="600" />

Current measurements are for the bare board, no LED or timer are attached.

