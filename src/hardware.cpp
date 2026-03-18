// hardware.cpp
#include "hardware.h"
#include "adc.h"
#include "vehicle_state.h"
#include "zephyr/drivers/can.h"
#include <cstdint>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/dt-bindings/gpio/gpio.h>

LOG_MODULE_REGISTER(hardware);

/* ========================================================================== */

int Hardware::init()
{
    LOG_INF("Initializing hardware...");

    // if (initializeADCs() != 0)
    // {
    //     LOG_ERR("Failed to initialize ADCs");
    //     return -1;
    // }

    if (initializeGPIOs() != 0)
    {
        LOG_ERR("Failed to initialize GPIOs");
        return -2;
    }

    if (initializePowerManager() != 0)
    {
        LOG_ERR("Failed to initialize GPIOs");
        return -3;
    }

    // if (initializeCANs() != 0)
    // {
    //     LOG_ERR("Failed to initialize CANs");
    //     return -3;
    // }

    LOG_INF("Hardware initialized successfully");
    return 0;
}

Hardware::Hardware(VehicleState &state)
{
    this->vehicle = state;
}

/* ========================================================================== */
/* ADC                                                                         */
/* ========================================================================== */

int Hardware::initializeADCs()
{
    LOG_INF("Initializing ADCs...");

    /* --- ADC1 ------------------------------------------------------------- */
    /*
     * DTS channel -> physical pin -> signal:
     *   channel@0 (INP18): PA4  -> I_10A_Limit
     *   channel@1 (INP19): PA5  -> I_5A_Limit
     *   channel@2 (INP5):  PB1  -> I_Pump2
     *   channel@3 (INP11): PC1  -> I_Rad1_Prot
     *   channel@4 (INP4):  PC4  -> I_Pump1
     *   channel@5 (INP8):  PC5  -> I_1a_Limit1
     *   channel@6 (INP10):  PC0  -> I_1a_Limit2
     *
     * init() takes the INP number, which must match
     * zephyr,input-positive in the DTS channel node.
     */
    adc1_dev_ = DEVICE_DT_GET(DT_NODELABEL(adc1));
    if (!device_is_ready(adc1_dev_))
    {
        LOG_ERR("ADC device adc1 not ready");
        return -1;
    }

    if (i_10a_limit.init(adc1_dev_, 18) != 0) /* PA4  - INP18 */
    {
        LOG_ERR("Failed to init i_10a_limit (PA4/INP18)");
        return -10;
    }
    if (i_5a_limit.init(adc1_dev_, 19) != 0) /* PA5  - INP19 */
    {
        LOG_ERR("Failed to init i_5a_limit (PA5/INP19)");
        return -11;
    }
    if (i_pump2.init(adc1_dev_, 5) != 0) /* PB1  - INP5  */
    {
        LOG_ERR("Failed to init i_pump2 (PB1/INP5)");
        return -12;
    }
    if (i_rad1_prot.init(adc1_dev_, 11) != 0) /* PC1  - INP11 */
    {
        LOG_ERR("Failed to init i_rad1_prot (PC1/INP11)");
        return -13;
    }
    if (i_pump1.init(adc1_dev_, 4) != 0) /* PC4  - INP4  */
    {
        LOG_ERR("Failed to init i_pump1 (PC4/INP4)");
        return -14;
    }
    if (i_1a_limit1.init(adc1_dev_, 8) != 0) /* PC5  - INP8  */
    {
        LOG_ERR("Failed to init i_1a_limit1 (PC5/INP8)");
        return -15;
    }
    if (i_1a_limit2.init(adc1_dev_, 10) != 0) /* PC0  - INP10  */
    {
        LOG_ERR("Failed to init i_1a_limit2 (PC0/INP10)");
        return -100;
    }

    /* --- ADC3 ------------------------------------------------------------- */
    /*
     *   channel@0 (INP0): PC2_C -> I_Rad2_Prot
     */
    // adc3_dev_ = DEVICE_DT_GET(DT_NODELABEL(adc3));
    // if (!device_is_ready(adc3_dev_))
    // {
    //     LOG_ERR("ADC device adc3 not ready");
    //     return -2;
    // }

    // if (i_rad2_prot.init(adc3_dev_, 0) != 0) /* PC2_C - INP0 */
    // {
    //     LOG_ERR("Failed to init i_rad2_prot (PC2_C/INP0)");
    //     return -16;
    // }

    LOG_INF("ADCs initialized");
    return 0;
}

/* ========================================================================== */
/* GPIO                                                                        */
/* ========================================================================== */

int Hardware::initializeGPIOs()
{
    LOG_INF("Initializing GPIOs...");

    /* --- Acquire GPIO port handles ---------------------------------------- */
    gpioa_ = DEVICE_DT_GET(DT_NODELABEL(gpioa));
    gpiob_ = DEVICE_DT_GET(DT_NODELABEL(gpiob));
    gpioc_ = DEVICE_DT_GET(DT_NODELABEL(gpioc));
    gpiod_ = DEVICE_DT_GET(DT_NODELABEL(gpiod));
    gpioe_ = DEVICE_DT_GET(DT_NODELABEL(gpioe));

    if (!device_is_ready(gpioa_) || !device_is_ready(gpiob_) || !device_is_ready(gpioc_) || !device_is_ready(gpiod_) ||
        !device_is_ready(gpioe_))
    {
        LOG_ERR("One or more GPIO ports not ready");
        return -1;
    }

    /* --- Status LEDs (PE2–PE6) -------------------------------------------- */
    if (led_yellow.init(gpioe_, 2, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("led_yellow");
        return -10;
    }
    if (led_orange.init(gpioe_, 3, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("led_orange");
        return -11;
    }
    if (led_red.init(gpioe_, 4, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("led_red");
        return -12;
    }
    if (led_blue.init(gpioe_, 5, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("led_blue");
        return -13;
    }
    if (led_green.init(gpioe_, 6, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("led_green");
        return -14;
    }

    /* --- Power gates ------------------------------------------------------- */
    /* GPIOA */
    if (spare_1a_gate.init(gpioa_, 8, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("spare_1a_gate");
        return -20;
    }

    /* GPIOB */
    if (spare_5a_gate2.init(gpiob_, 15, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("spare_5a_gate2");
        return -21;
    }
    if (bspd_gate.init(gpiob_, 14, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("bspd_gate");
        return -22;
    }
    if (dash_gate.init(gpiob_, 13, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("dash_gate");
        return -23;
    }
    if (pump1_gate.init(gpiob_, 12, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("pump1_gate");
        return -24;
    }
    if (rad1_gate.init(gpiob_, 11, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("rad1_gate");
        return -25;
    }
    if (fan4_gate.init(gpiob_, 10, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("fan4_gate");
        return -26;
    }
    if (pump2_gate.init(gpiob_, 2, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("pump2_gate");
        return -27;
    }

    /* GPIOC */
    if (spare_5a_gate.init(gpioc_, 9, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("spare_5a_gate");
        return -28;
    }

    /* GPIOD */
    if (spare_1a_gate2.init(gpiod_, 7, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("spare_1a_gate2");
        return -29;
    }
    if (strain_gate.init(gpiod_, 6, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("strain_gate");
        return -30;
    }
    if (emeter_gate.init(gpiod_, 5, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("emeter_gate");
        return -31;
    }
    if (bms_gate.init(gpiod_, 4, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("bms_gate");
        return -32;
    }
    if (imd_gate.init(gpiod_, 3, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("imd_gate");
        return -33;
    }
    if (sd_gate.init(gpiod_, 2, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("sd_gate");
        return -34;
    }
    if (rad2_gate.init(gpiod_, 1, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("rad2_gate");
        return -35;
    }

    /* GPIOE */
    if (fan3_gate.init(gpioe_, 15, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("fan3_gate");
        return -36;
    }
    if (fan2_gate.init(gpioe_, 14, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("fan2_gate");
        return -37;
    }
    if (vcu_gate.init(gpioe_, 12, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("vcu_gate");
        return -38;
    }
    if (fan1_gate.init(gpioe_, 10, GPIO_OUTPUT_INACTIVE) != 0)
    {
        LOG_ERR("fan1_gate");
        return -39;
    }

    /* --- Multiplexer select lines ----------------------------------------- */
    // if (mux_s1_0.init(gpioc_, 6, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s1_0");
    //     return -50;
    // }
    // if (mux_s1_1.init(gpiod_, 15, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s1_1");
    //     return -51;
    // }
    // if (mux_s2_0.init(gpiod_, 14, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s2_0");
    //     return -52;
    // }
    // if (mux_s2_1.init(gpiod_, 13, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s2_1");
    //     return -53;
    // }
    // if (mux_s3_0.init(gpiod_, 11, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s3_0");
    //     return -54;
    // }
    // if (mux_s3_1.init(gpiod_, 10, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s3_1");
    //     return -55;
    // }
    // if (mux_s4_1.init(gpioe_, 8, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s4_1");
    //     return -56;
    // }
    // if (mux_s4_0.init(gpioe_, 7, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("mux_s4_0");
    //     return -57;
    // }

    /* --- Debug GPIOs ------------------------------------------------------- */
    // if (debug1.init(gpiod_, 0, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("debug1");
    //     return -60;
    // }
    // if (debug2.init(gpioc_, 12, GPIO_OUTPUT_INACTIVE) != 0)
    // {
    //     LOG_ERR("debug2");
    //     return -61;
    // }

    LOG_INF("GPIOs initialized");
    return 0;
}

/* ========================================================================== */
/* CAN                                                                         */
/* ========================================================================== */

int Hardware::initializeCANs()
{
    LOG_INF("Initializing CANs...");

    can1_dev_ = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
    if (!device_is_ready(can1_dev_))
    {
        LOG_ERR("CAN device fdcan1 not ready");
        return -1;
    }

    if (can1.init(can1_dev_, 1000000, 875) != 0)
    {
        LOG_ERR("Failed to init CAN1");
        return -10;
    }

    if (can1.start() != 0)
    {
        LOG_ERR("Failed to start CAN1");
        return -11;
    }

    can1.set_mode(CAN_MODE_LOOPBACK);

    LOG_INF("CANs initialized");
    return 0;
}

/* ========================================================================== */
/* PowerManager                                                                */
/* ========================================================================== */

int Hardware::initializePowerManager()
{
    LOG_INF("Initializing power manager...");

    return power_mgr.init(
        /* gate pins */
        &pump1_gate, &pump2_gate, &rad1_gate, &rad2_gate, &fan1_gate, &fan2_gate, &fan3_gate, &fan4_gate, &bspd_gate,
        &dash_gate, &vcu_gate, &bms_gate, &imd_gate, &sd_gate, &emeter_gate, &strain_gate, &spare_1a_gate,
        &spare_1a_gate2, &spare_5a_gate, &spare_5a_gate2,

        /* direct ADC */
        &i_pump1, &i_pump2, &i_rad1_prot, &i_rad2_prot,

        /* MUX S1: select PC6(S0)/PD15(S1) → I_1a_Limit1 */
        &mux_s1_0, &mux_s1_1, &i_1a_limit1,

        /* MUX S2: select PD14(S0)/PD13(S1) → I_1A_Limit_2 */
        &mux_s2_0, &mux_s2_1, &i_1a_limit2,

        /* MUX S3: select PD11(S0)/PD10(S1) → I_5A_Limit */
        &mux_s3_0, &mux_s3_1, &i_5a_limit,

        /* MUX S4: select PE7(S0)/PE8(S1) → I_10A_Limit */
        &mux_s4_0, &mux_s4_1, &i_10a_limit);
}