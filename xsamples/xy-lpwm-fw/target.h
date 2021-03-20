#ifndef TARGET_BOARD_H
#define TARGET_BOARD_H

/*
 * XY-LPWM and clones pinout:
 *                                XY-LPWM
 *                          +------------------+
 *                       ---| 1 P0.5    P0.4 20|--- PWM3
 *              UART0 TX ---| 2 P0.6    P0.3 19|---
 *              UART0 RX ---| 3 P0.7    P0.2 18|--- ICPCLK [SCL]
 *                   RST ---| 4 P2.0    P0.1 17|--- LCD DATA
 *                 DUTY+ ---| 5 P3.0    P0.0 16|--- LCD /WR
 *                 DUTY- ---| 6 P1.7    P1.0 15|--- LCD /CS
 *                   GND ---| 7         P1.1 14|--- FREQ+
 * [SDA, ICPDA] DHT Data ---| 8 P1.6    P1.2 13|--- FREQ-
 *                   VDD ---| 9         P1.3 12|---
 *                       ---|10 P1.5    P1.4 11|---
 *                          +------------------+
 *
 *                                VHM-800
 *                          +------------------+
 *                       ---| 1 P0.5    P0.4 20|--- PWM3
 *              UART0 TX ---| 2 P0.6    P0.3 19|---
 *              UART0 RX ---| 3 P0.7    P0.2 18|--- ICPCLK [SCL]
 *                   RST ---| 4 P2.0    P0.1 17|--- LCD DATA
 *                 DUTY+ ---| 5 P3.0    P0.0 16|--- LCD /WR
 *                 DUTY- ---| 6 P1.7    P1.0 15|--- LCD /CS
 *                   GND ---| 7         P1.1 14|--- FREQ-
 * [SDA, ICPDA] DHT Data ---| 8 P1.6    P1.2 13|--- FREQ+
 *                   VDD ---| 9         P1.3 12|---
 *                       ---|10 P1.5    P1.4 11|---
 *                          +------------------+
 *
 *                                 NONAME
 *                          +------------------+
 *                  PWM2 ---| 1 P0.5    P0.4 20|---
 *              UART0 TX ---| 2 P0.6    P0.3 19|---
 *              UART0 RX ---| 3 P0.7    P0.2 18|--- ICPCLK [SCL]
 *                   RST ---| 4 P2.0    P0.1 17|--- LCD DATA
 *                       ---| 5 P3.0    P0.0 16|--- LCD /WR
 *                       ---| 6 P1.7    P1.0 15|--- LCD /CS
 *                   GND ---| 7         P1.1 14|--- DUTY-
 * [SDA, ICPDA] DHT Data ---| 8 P1.6    P1.2 13|--- DUTY+
 *                   VDD ---| 9         P1.3 12|--- FREQ-
 *                       ---|10 P1.5    P1.4 11|--- FREQ+
 *                          +------------------+
 */
#define XY_LPWM_BOARD 1
#define VHM_800_BOARD 2
#define NONAME_BOARD  3

/** assign TARGET_BOARD to one of the *_BOARD above to set the target board type */
#define TARGET_BOARD VHM_800_BOARD

/* select target board type */
#if (TARGET_BOARD == VHM_800_BOARD)
    #define PWM_CHANNEL 3
    #define PIN_FREQ_INC P12
    #define PIN_FREQ_DEC P11
    #define PIN_DUTY_INC P30
    #define PIN_DUTY_DEC P17
#elif (TARGET_BOARD == NONAME_BOARD)
    #define PWM_CHANNEL 2
    #define PIN_FREQ_INC P14
    #define PIN_FREQ_DEC P13
    #define PIN_DUTY_INC P12
    #define PIN_DUTY_DEC P11
#else /* by default XY-LPWM is selected */
    #define PWM_CHANNEL 3
    #define PIN_FREQ_INC P11
    #define PIN_FREQ_DEC P12
    #define PIN_DUTY_INC P30
    #define PIN_DUTY_DEC P17
#endif

/*
 * Originally XY-LPWM board has increment keys on the left and decrement on the right:
 * +-------+-------+-------+-------+
 * | FREQ+ | FREQ- | DUTY+ | DUTY- |
 * +-------+-------+-------+-------+
 *
 * set SWAP_KEYS to 1 to swap increment/decrement keys:
 * +-------+-------+-------+-------+
 * | FREQ- | FREQ+ | DUTY- | DUTY+ |
 * +-------+-------+-------+-------+
 */
#define SWAP_KEYS 1

#endif