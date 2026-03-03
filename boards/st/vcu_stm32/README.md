# Custom Power Distribution Module — `custom_pdm`

## Overview

The **Custom PDM** (Power Distribution Module) is a custom embedded controller board built around the **STM32H753VIT6** microcontroller (LQFP100 package). It is designed to manage power distribution, gate driving, current sensing, fan/pump PWM control, and CAN communication in an automotive or motorsport environment.

The board exposes a CAN bus interface, SWD/SWO debug port, multiple GPIO-controlled power gates, multiplexer select lines, ADC current sensing channels, PWM motor/fan outputs, and status indicator LEDs.

---

## SoC / Hardware

| Property        | Value                          |
|-----------------|-------------------------------|
| SoC             | STM32H753VIT6                 |
| Package         | LQFP100                       |
| Core            | ARM Cortex-M7 @ up to 480 MHz |
| Flash           | 2 MB                          |
| RAM             | 1 MB (including TCM)          |
| CAN             | FDCAN (via PB8/PB9)           |
| ADC             | ADC1/2 (12-bit), ADC3 (16-bit)|
| Timers (PWM)    | TIM1, TIM2, TIM3              |
| USART           | USART3                        |
| Debug           | SWD + SWO                     |
| Reset           | NRST, BOOT0                   |

---

## Supported Features

| Feature               | Config          | Notes                                   |
|-----------------------|-----------------|-----------------------------------------|
| CAN Bus (FDCAN)       | `fdcan1`        | PB8 (RX), PB9 (TX)                      |
| GPIO Power Gates      | `gpio`          | Multiple digital output gates (see below)|
| PWM Outputs           | `pwm`           | TIM1, TIM2, TIM3 channels               |
| ADC Current Sensing   | `adc`           | ADC1/2/3 channels                       |
| USART                 | `usart3`        | PD8 (TX), PD9 (RX)                      |
| SWD Debug             | `swd`           | PA13 (SWDIO), PA14 (SWCLK), PB3 (SWO)  |
| Status LEDs           | `gpio`          | 5× LEDs on PE2–PE6                      |
| MUX Select Lines      | `gpio`          | MUX S1–S4, 8 lines total                |

---

## Pin Mapping

### Debug & Boot

| Pin   | Signal  | Function          |
|-------|---------|-------------------|
| NRST  | NRST    | Reset             |
| BOOT0 | BOOT0   | Boot mode select  |
| PA13  | SWDIO   | SWD Data          |
| PA14  | SWCLK   | SWD Clock         |
| PB3   | SWO     | Serial Wire Output|
| PD0   | Debug1  | GPIO debug output |
| PC12  | Debug2  | GPIO debug output |

### CAN Bus

| Pin  | Signal       | Peripheral     |
|------|--------------|----------------|
| PB9  | CAN Transmit | FDCAN1_TX      |
| PB8  | CAN Receive  | FDCAN1_RX      |

### USART

| Pin  | Signal     | Peripheral    |
|------|------------|---------------|
| PD8  | USART3 TX  | USART3_TX     |
| PD9  | USART3 RX  | USART3_RX     |

### PWM Outputs

| Pin  | Signal        | Peripheral   |
|------|---------------|--------------|
| PE9  | Rad 2 PWM     | TIM1_CH1     |
| PE11 | Pump 1 PWM    | TIM1_CH2     |
| PE13 | Pump 2 PWM    | TIM1_CH4     |
| PA0  | Fan 1 PWM     | TIM2_CH1     |
| PA2  | Fan 2 PWM     | TIM2_CH3     |
| PA6  | Fan 3 PWM     | TIM3_CH1     |
| PA7  | Rad 1 PWM     | TIM3_CH2     |
| PB0  | Fan 4 PWM     | TIM3_CH3     |

### ADC Current Sensing

| Pin    | Signal        | Peripheral       |
|--------|---------------|------------------|
| PB1    | I_Pump2       | ADC12_INP5       |
| PC5    | I_1a_Limit1   | ADC12_INP8       |
| PC4    | I_Pump1       | ADC12_INP4       |
| PA5    | I_5A_Limit    | ADC12_INP19      |
| PA4    | I_10A_Limit   | ADC12_INP18      |
| PC2_C  | I_Rad2_Prot   | ADC3_INP0        |
| PC1    | I_Rad1_Prot   | ADC123_INP11     |

### Power Gate Outputs (Digital GPIO — Output)

| Pin  | Signal            | Description                        |
|------|-------------------|------------------------------------|
| PD7  | Spare 1A Gate 2   | Spare 1 A power gate (secondary)   |
| PD6  | Strain Gate       | Strain sensor supply gate          |
| PD5  | EMeter Gate       | Energy meter supply gate           |
| PD4  | BMS Gate          | Battery Management System gate     |
| PD3  | IMD Gate          | Insulation Monitoring Device gate  |
| PD2  | SD Gate           | Shutdown circuit gate              |
| PD1  | Rad2 Gate         | Radiator 2 supply gate             |
| PA8  | Spare 1A Gate     | Spare 1 A power gate (primary)     |
| PC9  | Spare 5A Gate     | Spare 5 A power gate (primary)     |
| PB15 | Spare 5A Gate 2   | Spare 5 A power gate (secondary)   |
| PB14 | BSPD Gate         | Brake System Plausibility Device gate |
| PB13 | Dash Gate         | Dashboard supply gate              |
| PB12 | Pump 1 Gate       | Pump 1 enable gate                 |
| PB11 | Rad 1 Gate        | Radiator 1 supply gate             |
| PB10 | Fan 4 Gate        | Fan 4 enable gate                  |
| PE15 | Fan 3 Gate        | Fan 3 enable gate                  |
| PE14 | Fan 2 Gate        | Fan 2 enable gate                  |
| PE12 | VCU Gate          | Vehicle Control Unit supply gate   |
| PE10 | Fan 1 Gate        | Fan 1 enable gate                  |
| PB2  | Pump 2 Gate       | Pump 2 enable gate                 |

### Multiplexer Select Lines (Digital GPIO — Output)

| Pin  | Signal    | Description              |
|------|-----------|--------------------------|
| PC6  | MUX S1-0  | MUX 1 select bit 0       |
| PD15 | MUX S1-1  | MUX 1 select bit 1       |
| PD14 | MUX S2-0  | MUX 2 select bit 0       |
| PD13 | MUX S2-1  | MUX 2 select bit 1       |
| PD11 | MUX S3-0  | MUX 3 select bit 0       |
| PD10 | MUX S3-1  | MUX 3 select bit 1       |
| PE8  | MUX S4-1  | MUX 4 select bit 1       |
| PE7  | MUX SR-0  | MUX SR select bit 0      |

### Status LEDs (Digital GPIO — Output)

| Pin  | Signal           | Colour  |
|------|------------------|---------|
| PE6  | ERR_LED_GREEN    | Green   |
| PE5  | ERR_LED_BLUE     | Blue    |
| PE4  | ERR_LED_RED      | Red     |
| PE3  | ERR_LED_ORANGE   | Orange  |
| PE2  | ERR_LED_YELLOW   | Yellow  |

---

## Board Directory Structure

When integrating this board into a Zephyr workspace, the expected directory layout is:

```
boards/custom/custom_pdm/
├── board.yml
├── Kconfig.custom_pdm
├── Kconfig.defconfig
├── custom_pdm.dts
├── custom_pdm_defconfig
└── custom_pdm-pinctrl.dtsi
```

---

## Building

```bash
west build -b custom_pdm/stm32h753vi -- path/to/your/app
```

---

## Flashing / Debugging

The board exposes a standard **SWD** interface (PA13/PA14) with an additional **SWO** trace pin (PB3). Any ST-Link v2/v3 or J-Link probe may be used.

```bash
west flash
west debug
```

---

## References

- [STM32H753VIT6 Product Page](https://www.st.com/en/microcontrollers-microprocessors/stm32h753vi.html)
- [STM32H7 Reference Manual (RM0433)](https://www.st.com/resource/en/reference_manual/rm0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Zephyr Board Porting Guide](https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html)
- [STM32H7 Zephyr SoC Support](https://docs.zephyrproject.org/latest/boards/st/)
