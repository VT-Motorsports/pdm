#pragma once

// power_manager.h
//
// Owns every PROFET channel and the four analog multiplexers on the PDM.
//
// Mux routing
// ───────────
//  MUX S1  select: PC6(S0) / PD15(S1)   ADC output: PC5  / I_1a_Limit1 (INP8)
//    ch 0 (00) → IMD current
//    ch 1 (01) → BSPD current
//    ch 2 (10) → BMS current
//    ch 3 (11) → Spare 1A current
//
//  MUX S2  select: PD14(S0) / PD13(S1)  ADC output: PC0 / I_1A_Limit_2  (INP10)
//    ch 0 (00) → VCU current
//    ch 1 (01) → EMeter current
//    ch 2 (10) → Spare 1A 2 current
//    ch 3 (11) → Strain current
//
//  MUX S3  select: PD11(S0) / PD10(S1)  ADC output: PA5  / I_5A_Limit  (INP19)
//    ch 0 (00) → Dash current
//    ch 1 (01) → Spare 5A current
//    ch 2 (10) → Spare 5A 2 current
//    ch 3 (11) → SD current
//
//  MUX S4  select: PE7(S0)  / PE8(S1)   ADC output: PA4  / I_10A_Limit (INP18)
//    ch 0 (00) → Fan 1 current
//    ch 1 (01) → Fan 2 current
//    ch 2 (10) → Fan 3 current
//    ch 3 (11) → Fan 4 current
//
// Direct ADC (no mux):
//   pump1  → PC4 / I_Pump1  (ADC1_INP4)
//   pump2  → PB1 / I_Pump2  (ADC1_INP5)
//   rad1   → PC1 / I_Rad1_Prot (ADC1_INP11)
//   rad2   → PC2_C / I_Rad2_Prot (ADC3_INP0)
//

#include "adc.h"
#include "gpio.h"
#include "mux_adc.h"
#include "profet.h"
#include <cstdint>

/* -------------------------------------------------------------------------- */
/* CAN-driven enable flags (3 bytes, 1 bit per channel)                       */
/* -------------------------------------------------------------------------- */
struct PdmFlags
{
    /* Pumps & radiators */
    uint8_t pump1 : 1;
    uint8_t pump2 : 1;
    uint8_t rad1 : 1;
    uint8_t rad2 : 1;

    /* Fan gates (PWM duty is controlled separately via TIM) */
    uint8_t fan1 : 1;
    uint8_t fan2 : 1;
    uint8_t fan3 : 1;
    uint8_t fan4 : 1;

    /* Safety / control */
    uint8_t bspd : 1;
    uint8_t dash : 1;
    uint8_t vcu : 1;
    uint8_t bms : 1;
    uint8_t imd : 1;
    uint8_t sd : 1;

    /* Sensing / ancillary */
    uint8_t emeter : 1;
    uint8_t strain : 1;

    /* Spare outputs */
    uint8_t spare_1a : 1;
    uint8_t spare_1a_2 : 1;
    uint8_t spare_5a : 1;
    uint8_t spare_5a_2 : 1;

    uint8_t _reserved : 4;
};
static_assert(sizeof(PdmFlags) == 3, "PdmFlags must be 3 bytes");

/* -------------------------------------------------------------------------- */
/* PowerManager                                                                */
/* -------------------------------------------------------------------------- */
class PowerManager
{
  public:
    PowerManager() = default;

    /* --------------------------------------------------------------------- */
    /* Optional kILIS overrides — call before init()                         */
    /* --------------------------------------------------------------------- */
    void set_k_ilis_pump1(float k)
    {
        k_pump1_ = k;
    }
    void set_k_ilis_pump2(float k)
    {
        k_pump2_ = k;
    }
    void set_k_ilis_rad1(float k)
    {
        k_rad1_ = k;
    }
    void set_k_ilis_rad2(float k)
    {
        k_rad2_ = k;
    }
    void set_k_ilis_fans(float k)
    {
        k_fans_ = k;
    }
    void set_k_ilis_5a(float k)
    {
        k_5a_ = k;
    }
    void set_k_ilis_1a(float k)
    {
        k_1a_ = k;
    }

    /* --------------------------------------------------------------------- */
    /* Initialisation                                                         */
    /*                                                                        */
    /* All GpioPin and AdcChannel objects must already be init()'d before    */
    /* calling this.  Pass the mux select pins and ADC outputs; PowerManager */
    /* builds the MuxAdcChannel objects internally.                          */
    /* --------------------------------------------------------------------- */
    int init(
        /* --- Gate pins ---------------------------------------------------- */
        GpioPin *gate_pump1, GpioPin *gate_pump2, GpioPin *gate_rad1, GpioPin *gate_rad2, GpioPin *gate_fan1,
        GpioPin *gate_fan2, GpioPin *gate_fan3, GpioPin *gate_fan4, GpioPin *gate_bspd, GpioPin *gate_dash,
        GpioPin *gate_vcu, GpioPin *gate_bms, GpioPin *gate_imd, GpioPin *gate_sd, GpioPin *gate_emeter,
        GpioPin *gate_strain, GpioPin *gate_spare_1a, GpioPin *gate_spare_1a_2, GpioPin *gate_spare_5a,
        GpioPin *gate_spare_5a_2,

        /* --- Direct ADC (no mux) ------------------------------------------ */
        AdcChannel *adc_pump1, /* PC4  / INP4  */
        AdcChannel *adc_pump2, /* PB1  / INP5  */
        AdcChannel *adc_rad1,  /* PC1  / INP11 */
        AdcChannel *adc_rad2,  /* PC2_C/ INP0  */

        /* --- Mux select GPIO pairs ---------------------------------------- */
        /* S1 */ GpioPin *s1_s0, GpioPin *s1_s1, AdcChannel *adc_s1_out,
        /* S2 */ GpioPin *s2_s0, GpioPin *s2_s1, AdcChannel *adc_s2_out,
        /* S3 */ GpioPin *s3_s0, GpioPin *s3_s1, AdcChannel *adc_s3_out,
        /* S4 */ GpioPin *s4_s0, GpioPin *s4_s1, AdcChannel *adc_s4_out);

    /* --------------------------------------------------------------------- */
    /* Flag-driven control — call from CAN receive handler                   */
    /* --------------------------------------------------------------------- */
    int apply_flags(const PdmFlags &flags);

    /* --------------------------------------------------------------------- */
    /* Bulk helpers                                                           */
    /* --------------------------------------------------------------------- */
    int enable_all();
    int disable_all();
    void log_currents();

    /* --------------------------------------------------------------------- */
    /* Per-channel accessors                                                  */
    /* --------------------------------------------------------------------- */
    Profet &pump1()
    {
        return pump1_;
    }
    Profet &pump2()
    {
        return pump2_;
    }
    Profet &rad1()
    {
        return rad1_;
    }
    Profet &rad2()
    {
        return rad2_;
    }
    Profet &fan1()
    {
        return fan1_;
    }
    Profet &fan2()
    {
        return fan2_;
    }
    Profet &fan3()
    {
        return fan3_;
    }
    Profet &fan4()
    {
        return fan4_;
    }
    Profet &bspd()
    {
        return bspd_;
    }
    Profet &dash()
    {
        return dash_;
    }
    Profet &vcu()
    {
        return vcu_;
    }
    Profet &bms()
    {
        return bms_;
    }
    Profet &imd()
    {
        return imd_;
    }
    Profet &sd()
    {
        return sd_;
    }
    Profet &emeter()
    {
        return emeter_;
    }
    Profet &strain()
    {
        return strain_;
    }
    Profet &spare_1a()
    {
        return spare_1a_;
    }
    Profet &spare_1a_2()
    {
        return spare_1a_2_;
    }
    Profet &spare_5a()
    {
        return spare_5a_;
    }
    Profet &spare_5a_2()
    {
        return spare_5a_2_;
    }

    /* Mux accessors (for diagnostics / manual reads) */
    MuxAdcChannel &mux_s1()
    {
        return mux_s1_;
    }
    MuxAdcChannel &mux_s2()
    {
        return mux_s2_;
    }
    MuxAdcChannel &mux_s3()
    {
        return mux_s3_;
    }
    MuxAdcChannel &mux_s4()
    {
        return mux_s4_;
    }

  private:
    /* kILIS values — defaults applied in init() if not overridden */
    float k_pump1_ = 0.0f;
    float k_pump2_ = 0.0f;
    float k_rad1_ = 0.0f;
    float k_rad2_ = 0.0f;
    float k_fans_ = 0.0f; /* applied to fan1-4   */
    float k_5a_ = 0.0f;   /* applied to dash, spare5a*, sd */
    float k_1a_ = 0.0f;   /* applied to imd, bspd, bms, spare1a*, vcu, emeter, strain */

    /* Multiplexers */
    MuxAdcChannel mux_s1_;
    MuxAdcChannel mux_s2_;
    MuxAdcChannel mux_s3_;
    MuxAdcChannel mux_s4_;

    /* PROFET channels */
    /* Direct sense */
    Profet pump1_{"pump1"};
    Profet pump2_{"pump2"};
    Profet rad1_{"rad1"};
    Profet rad2_{"rad2"};
    /* MUX S1 */
    Profet imd_{"imd"};
    Profet bspd_{"bspd"};
    Profet bms_{"bms"};
    Profet spare_1a_{"spare_1a"};
    /* MUX S2 */
    Profet vcu_{"vcu"};
    Profet emeter_{"emeter"};
    Profet spare_1a_2_{"spare_1a_2"};
    Profet strain_{"strain"};
    /* MUX S3 */
    Profet dash_{"dash"};
    Profet spare_5a_{"spare_5a"};
    Profet spare_5a_2_{"spare_5a_2"};
    Profet sd_{"sd"};
    /* MUX S4 */
    Profet fan1_{"fan1"};
    Profet fan2_{"fan2"};
    Profet fan3_{"fan3"};
    Profet fan4_{"fan4"};

    bool initialized_{false};

    static int apply_one(Profet &ch, bool enable);
};