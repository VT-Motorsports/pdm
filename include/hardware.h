#pragma once

// hardware.h
#include "adc.h"
#include "can.h"
#include "gpio.h" // assumed wrapper matching the .init(port, pin, flags) API
#include "power_manager.h"
#include "vehicle_state.h"
#include <zephyr/device.h>

class Hardware
{
  public:
    Hardware(VehicleState &state);
    int init();

    /* ------------------------------------------------------------------ */
    /* ADC channels                                                        */
    /* ------------------------------------------------------------------ */
    AdcChannel i_10a_limit; /* PA4  - ADC1_INP18 */
    AdcChannel i_5a_limit;  /* PA5  - ADC1_INP19 */
    AdcChannel i_pump2;     /* PB1  - ADC1_INP5  */
    AdcChannel i_rad1_prot; /* PC1  - ADC1_INP11 */
    AdcChannel i_pump1;     /* PC4  - ADC1_INP4  */
    AdcChannel i_1a_limit1; /* PC5  - ADC1_INP8  */
    AdcChannel i_1a_limit2; /* PC0  - ADC123_INP10  */
    AdcChannel i_rad2_prot; /* PC2_C- ADC3_INP0  */

    /* ------------------------------------------------------------------ */
    /* Status LEDs                                                         */
    /* ------------------------------------------------------------------ */
    GpioPin led_yellow; /* PE2 */
    GpioPin led_orange; /* PE3 */
    GpioPin led_red;    /* PE4 */
    GpioPin led_blue;   /* PE5 */
    GpioPin led_green;  /* PE6 */

    /* ------------------------------------------------------------------ */
    /* Power gate outputs                                                  */
    /* ------------------------------------------------------------------ */
    GpioPin spare_1a_gate;  /* PA8  */
    GpioPin spare_5a_gate2; /* PB15 */
    GpioPin bspd_gate;      /* PB14 */
    GpioPin dash_gate;      /* PB13 */
    GpioPin pump1_gate;     /* PB12 */
    GpioPin rad1_gate;      /* PB11 */
    GpioPin fan4_gate;      /* PB10 */
    GpioPin pump2_gate;     /* PB2  */
    GpioPin spare_5a_gate;  /* PC9  */
    GpioPin spare_1a_gate2; /* PD7  */
    GpioPin strain_gate;    /* PD6  */
    GpioPin emeter_gate;    /* PD5  */
    GpioPin bms_gate;       /* PD4  */
    GpioPin imd_gate;       /* PD3  */
    GpioPin sd_gate;        /* PD2  */
    GpioPin rad2_gate;      /* PD1  */
    GpioPin fan3_gate;      /* PE15 */
    GpioPin fan2_gate;      /* PE14 */
    GpioPin vcu_gate;       /* PE12 */
    GpioPin fan1_gate;      /* PE10 */

    /* ------------------------------------------------------------------ */
    /* Multiplexer select lines                                            */
    /* ------------------------------------------------------------------ */
    GpioPin mux_s1_0; /* PC6  */
    GpioPin mux_s1_1; /* PD15 */
    GpioPin mux_s2_0; /* PD14 */
    GpioPin mux_s2_1; /* PD13 */
    GpioPin mux_s3_0; /* PD11 */
    GpioPin mux_s3_1; /* PD10 */
    GpioPin mux_s4_1; /* PE8  */
    GpioPin mux_s4_0; /* PE7  */

    /* ------------------------------------------------------------------ */
    /* Debug GPIOs                                                         */
    /* ------------------------------------------------------------------ */
    GpioPin debug1; /* PD0  */
    GpioPin debug2; /* PC12 */

    /* ------------------------------------------------------------------ */
    /* CAN                                                                 */
    /* ------------------------------------------------------------------ */
    CanBus can1;

    /* ------------------------------------------------------------------ */
    /* Power management                                                    */
    /* ------------------------------------------------------------------ */
    PowerManager power_mgr;

  private:
    int initializeADCs();
    int initializeGPIOs();
    int initializeCANs();
    int initializePowerManager();

    /* GPIO port handles */
    const device *gpioa_{nullptr};
    const device *gpiob_{nullptr};
    const device *gpioc_{nullptr};
    const device *gpiod_{nullptr};
    const device *gpioe_{nullptr};

    /* ADC device handles */
    const device *adc1_dev_{nullptr};
    const device *adc3_dev_{nullptr};

    /* CAN device handle */
    const device *can1_dev_{nullptr};

    VehicleState vehicle;
};
