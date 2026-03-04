// power_manager.cpp
#include "power_manager.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(power_manager);

/* ========================================================================== */
/* Default kILIS values                                                        */
/*                                                                             */
/* Update these to match the PROFET part numbers on your BOM.                 */
/* "Sense Current Ratio" table in each PROFET datasheet.                      */
/*   BTS7002-1EPP  : kILIS = 20 000                                           */
/*   BTS7004-1EPP  : kILIS = 20 000                                           */
/*   BTS7008-2EPA  : kILIS = 22 700                                           */
/*   BTS3008        : kILIS = 14 000  (example for 1 A parts)                 */
/* ========================================================================== */
static constexpr float K_ILIS_PUMP = 20000.0f;
static constexpr float K_ILIS_RAD = 20000.0f;
static constexpr float K_ILIS_10A = 20000.0f; /* fan1-4  via MUX S4 */
static constexpr float K_ILIS_5A = 14000.0f;  /* dash, spare5a*, sd via MUX S3 */
static constexpr float K_ILIS_1A = 14000.0f;  /* imd, bspd, bms, spare1a*, vcu,
                                                   emeter, strain via MUX S1/S2 */

/* ========================================================================== */
/* init()                                                                      */
/* ========================================================================== */

int PowerManager::init(GpioPin *gate_pump1, GpioPin *gate_pump2, GpioPin *gate_rad1, GpioPin *gate_rad2,
                       GpioPin *gate_fan1, GpioPin *gate_fan2, GpioPin *gate_fan3, GpioPin *gate_fan4,
                       GpioPin *gate_bspd, GpioPin *gate_dash, GpioPin *gate_vcu, GpioPin *gate_bms, GpioPin *gate_imd,
                       GpioPin *gate_sd, GpioPin *gate_emeter, GpioPin *gate_strain, GpioPin *gate_spare_1a,
                       GpioPin *gate_spare_1a_2, GpioPin *gate_spare_5a, GpioPin *gate_spare_5a_2,
                       AdcChannel *adc_pump1, AdcChannel *adc_pump2, AdcChannel *adc_rad1, AdcChannel *adc_rad2,
                       GpioPin *s1_s0, GpioPin *s1_s1, AdcChannel *adc_s1_out, GpioPin *s2_s0, GpioPin *s2_s1,
                       AdcChannel *adc_s2_out, GpioPin *s3_s0, GpioPin *s3_s1, AdcChannel *adc_s3_out, GpioPin *s4_s0,
                       GpioPin *s4_s1, AdcChannel *adc_s4_out)
{
    LOG_INF("Initializing power manager...");

    /* Apply kILIS defaults for any group not manually overridden */
    if (k_pump1_ == 0.0f)
        k_pump1_ = K_ILIS_PUMP;
    if (k_pump2_ == 0.0f)
        k_pump2_ = K_ILIS_PUMP;
    if (k_rad1_ == 0.0f)
        k_rad1_ = K_ILIS_RAD;
    if (k_rad2_ == 0.0f)
        k_rad2_ = K_ILIS_RAD;
    if (k_fans_ == 0.0f)
        k_fans_ = K_ILIS_10A;
    if (k_5a_ == 0.0f)
        k_5a_ = K_ILIS_5A;
    if (k_1a_ == 0.0f)
        k_1a_ = K_ILIS_1A;

    int ret = 0;

    /* ------------------------------------------------------------------ */
    /* Build multiplexers                                                   */
    /* ------------------------------------------------------------------ */
    if ((ret = mux_s1_.init(s1_s0, s1_s1, adc_s1_out, "S1")) != 0)
        return ret;
    if ((ret = mux_s2_.init(s2_s0, s2_s1, adc_s2_out, "S2")) != 0)
        return ret;
    if ((ret = mux_s3_.init(s3_s0, s3_s1, adc_s3_out, "S3")) != 0)
        return ret;
    if ((ret = mux_s4_.init(s4_s0, s4_s1, adc_s4_out, "S4")) != 0)
        return ret;

    /* ------------------------------------------------------------------ */
    /* Direct-sense channels (pump / rad — own dedicated ADC lines)        */
    /* ------------------------------------------------------------------ */
    pump1_ = Profet{"pump1", k_pump1_};
    pump2_ = Profet{"pump2", k_pump2_};
    rad1_ = Profet{"rad1", k_rad1_};
    rad2_ = Profet{"rad2", k_rad2_};

    if ((ret = pump1_.init(gate_pump1, adc_pump1)) != 0)
    {
        LOG_ERR("pump1 init");
        return ret;
    }
    if ((ret = pump2_.init(gate_pump2, adc_pump2)) != 0)
    {
        LOG_ERR("pump2 init");
        return ret;
    }
    if ((ret = rad1_.init(gate_rad1, adc_rad1)) != 0)
    {
        LOG_ERR("rad1 init");
        return ret;
    }
    if ((ret = rad2_.init(gate_rad2, adc_rad2)) != 0)
    {
        LOG_ERR("rad2 init");
        return ret;
    }

    /* ------------------------------------------------------------------ */
    /* MUX S1 — I_1a_Limit1 (PC5/INP8)                                    */
    /*   ch0=imd  ch1=bspd  ch2=bms  ch3=spare_1a                         */
    /* ------------------------------------------------------------------ */
    imd_ = Profet{"imd", k_1a_};
    bspd_ = Profet{"bspd", k_1a_};
    bms_ = Profet{"bms", k_1a_};
    spare_1a_ = Profet{"spare_1a", k_1a_};

    if ((ret = imd_.init(gate_imd, &mux_s1_, 0)) != 0)
    {
        LOG_ERR("imd init");
        return ret;
    }
    if ((ret = bspd_.init(gate_bspd, &mux_s1_, 1)) != 0)
    {
        LOG_ERR("bspd init");
        return ret;
    }
    if ((ret = bms_.init(gate_bms, &mux_s1_, 2)) != 0)
    {
        LOG_ERR("bms init");
        return ret;
    }
    if ((ret = spare_1a_.init(gate_spare_1a, &mux_s1_, 3)) != 0)
    {
        LOG_ERR("spare_1a init");
        return ret;
    }

    /* ------------------------------------------------------------------ */
    /* MUX S2 — I_1A_Limit_2 (**see note in header**)                     */
    /*   ch0=vcu  ch1=emeter  ch2=spare_1a_2  ch3=strain                  */
    /* ------------------------------------------------------------------ */
    vcu_ = Profet{"vcu", k_1a_};
    emeter_ = Profet{"emeter", k_1a_};
    spare_1a_2_ = Profet{"spare_1a_2", k_1a_};
    strain_ = Profet{"strain", k_1a_};

    if ((ret = vcu_.init(gate_vcu, &mux_s2_, 0)) != 0)
    {
        LOG_ERR("vcu init");
        return ret;
    }
    if ((ret = emeter_.init(gate_emeter, &mux_s2_, 1)) != 0)
    {
        LOG_ERR("emeter init");
        return ret;
    }
    if ((ret = spare_1a_2_.init(gate_spare_1a_2, &mux_s2_, 2)) != 0)
    {
        LOG_ERR("spare_1a_2 init");
        return ret;
    }
    if ((ret = strain_.init(gate_strain, &mux_s2_, 3)) != 0)
    {
        LOG_ERR("strain init");
        return ret;
    }

    /* ------------------------------------------------------------------ */
    /* MUX S3 — I_5A_Limit (PA5/INP19)                                    */
    /*   ch0=dash  ch1=spare_5a  ch2=spare_5a_2  ch3=sd                   */
    /* ------------------------------------------------------------------ */
    dash_ = Profet{"dash", k_5a_};
    spare_5a_ = Profet{"spare_5a", k_5a_};
    spare_5a_2_ = Profet{"spare_5a_2", k_5a_};
    sd_ = Profet{"sd", k_5a_};

    if ((ret = dash_.init(gate_dash, &mux_s3_, 0)) != 0)
    {
        LOG_ERR("dash init");
        return ret;
    }
    if ((ret = spare_5a_.init(gate_spare_5a, &mux_s3_, 1)) != 0)
    {
        LOG_ERR("spare_5a init");
        return ret;
    }
    if ((ret = spare_5a_2_.init(gate_spare_5a_2, &mux_s3_, 2)) != 0)
    {
        LOG_ERR("spare_5a_2 init");
        return ret;
    }
    if ((ret = sd_.init(gate_sd, &mux_s3_, 3)) != 0)
    {
        LOG_ERR("sd init");
        return ret;
    }

    /* ------------------------------------------------------------------ */
    /* MUX S4 — I_10A_Limit (PA4/INP18)                                   */
    /*   ch0=fan1  ch1=fan2  ch2=fan3  ch3=fan4                           */
    /* ------------------------------------------------------------------ */
    fan1_ = Profet{"fan1", k_fans_};
    fan2_ = Profet{"fan2", k_fans_};
    fan3_ = Profet{"fan3", k_fans_};
    fan4_ = Profet{"fan4", k_fans_};

    if ((ret = fan1_.init(gate_fan1, &mux_s4_, 0)) != 0)
    {
        LOG_ERR("fan1 init");
        return ret;
    }
    if ((ret = fan2_.init(gate_fan2, &mux_s4_, 1)) != 0)
    {
        LOG_ERR("fan2 init");
        return ret;
    }
    if ((ret = fan3_.init(gate_fan3, &mux_s4_, 2)) != 0)
    {
        LOG_ERR("fan3 init");
        return ret;
    }
    if ((ret = fan4_.init(gate_fan4, &mux_s4_, 3)) != 0)
    {
        LOG_ERR("fan4 init");
        return ret;
    }

    initialized_ = true;
    LOG_INF("Power manager initialized — all 20 channels ready");
    return 0;
}

/* ========================================================================== */
/* apply_flags()                                                               */
/* ========================================================================== */

int PowerManager::apply_flags(const PdmFlags &flags)
{
    if (!initialized_)
    {
        LOG_ERR("PowerManager not initialized");
        return -1;
    }

    int ret = 0;
    ret |= apply_one(pump1_, flags.pump1);
    ret |= apply_one(pump2_, flags.pump2);
    ret |= apply_one(rad1_, flags.rad1);
    ret |= apply_one(rad2_, flags.rad2);
    ret |= apply_one(fan1_, flags.fan1);
    ret |= apply_one(fan2_, flags.fan2);
    ret |= apply_one(fan3_, flags.fan3);
    ret |= apply_one(fan4_, flags.fan4);
    ret |= apply_one(bspd_, flags.bspd);
    ret |= apply_one(dash_, flags.dash);
    ret |= apply_one(vcu_, flags.vcu);
    ret |= apply_one(bms_, flags.bms);
    ret |= apply_one(imd_, flags.imd);
    ret |= apply_one(sd_, flags.sd);
    ret |= apply_one(emeter_, flags.emeter);
    ret |= apply_one(strain_, flags.strain);
    ret |= apply_one(spare_1a_, flags.spare_1a);
    ret |= apply_one(spare_1a_2_, flags.spare_1a_2);
    ret |= apply_one(spare_5a_, flags.spare_5a);
    ret |= apply_one(spare_5a_2_, flags.spare_5a_2);
    return ret;
}

/* ========================================================================== */
/* disable_all()                                                               */
/* ========================================================================== */

int PowerManager::disable_all()
{
    LOG_WRN("Disabling all PROFET channels");
    int ret = 0;
    ret |= pump1_.disable();
    ret |= pump2_.disable();
    ret |= rad1_.disable();
    ret |= rad2_.disable();
    ret |= fan1_.disable();
    ret |= fan2_.disable();
    ret |= fan3_.disable();
    ret |= fan4_.disable();
    ret |= bspd_.disable();
    ret |= dash_.disable();
    ret |= vcu_.disable();
    ret |= bms_.disable();
    ret |= imd_.disable();
    ret |= sd_.disable();
    ret |= emeter_.disable();
    ret |= strain_.disable();
    ret |= spare_1a_.disable();
    ret |= spare_1a_2_.disable();
    ret |= spare_5a_.disable();
    ret |= spare_5a_2_.disable();
    return ret;
}

/* ========================================================================== */
/* log_currents()                                                              */
/* ========================================================================== */

void PowerManager::log_currents()
{
    LOG_INF("=== PDM current readings ===");

    /* Helper lambda — avoids repeating the format string 20 times */
    auto log_ch = [](Profet &ch) {
        LOG_INF("  %-14s  %s  %.3f A", ch.name(), ch.is_enabled() ? "ON " : "OFF", (double)ch.read_current_amps());
    };

    LOG_INF("-- Direct sense --");
    log_ch(pump1_);
    log_ch(pump2_);
    log_ch(rad1_);
    log_ch(rad2_);

    /* Note: muxed channels share one ADC line so they are read sequentially.
       Reading all four on the same mux takes 4 × SETTLE_US each.            */
    LOG_INF("-- MUX S1 (I_1a_Limit1) --");
    log_ch(imd_);
    log_ch(bspd_);
    log_ch(bms_);
    log_ch(spare_1a_);

    LOG_INF("-- MUX S2 (I_1A_Limit_2) --");
    log_ch(vcu_);
    log_ch(emeter_);
    log_ch(spare_1a_2_);
    log_ch(strain_);

    LOG_INF("-- MUX S3 (I_5A_Limit) --");
    log_ch(dash_);
    log_ch(spare_5a_);
    log_ch(spare_5a_2_);
    log_ch(sd_);

    LOG_INF("-- MUX S4 (I_10A_Limit) --");
    log_ch(fan1_);
    log_ch(fan2_);
    log_ch(fan3_);
    log_ch(fan4_);
}

/* ========================================================================== */
/* apply_one() — private                                                       */
/* ========================================================================== */

int PowerManager::apply_one(Profet &ch, bool enable)
{
    if (enable == ch.is_enabled())
        return 0;
    return enable ? ch.enable() : ch.disable();
}
