// profet.cpp
#include "profet.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(profet);

/* ========================================================================== */

Profet::Profet(const char *name, float k_ilis, float r_sense) : name_(name), k_ilis_(k_ilis), r_sense_(r_sense)
{
}

/* ========================================================================== */
/* Initialisation                                                              */
/* ========================================================================== */

int Profet::init(GpioPin *gate)
{
    if (!gate)
    {
        LOG_ERR("[%s] gate pin is NULL", name_);
        return -1;
    }
    gate_ = gate;
    sense_mode_ = SenseMode::NONE;
    initialized_ = true;
    LOG_INF("[%s] initialized (no current sense)", name_);
    return 0;
}

int Profet::init(GpioPin *gate, AdcChannel *adc)
{
    if (!gate || !adc)
    {
        LOG_ERR("[%s] NULL argument in init(gate, adc)", name_);
        return -1;
    }
    gate_ = gate;
    direct_adc_ = adc;
    sense_mode_ = SenseMode::DIRECT;
    initialized_ = true;
    LOG_INF("[%s] initialized (direct ADC sense)", name_);
    return 0;
}

int Profet::init(GpioPin *gate, MuxAdcChannel *mux, uint8_t mux_channel)
{
    if (!gate || !mux)
    {
        LOG_ERR("[%s] NULL argument in init(gate, mux, ch)", name_);
        return -1;
    }
    if (mux_channel > 3U)
    {
        LOG_ERR("[%s] mux_channel %u out of range", name_, mux_channel);
        return -2;
    }
    gate_ = gate;
    mux_ = mux;
    mux_channel_ = mux_channel;
    sense_mode_ = SenseMode::MUXED;
    initialized_ = true;
    LOG_INF("[%s] initialized (mux '%s' ch%u)", name_, mux->name(), mux_channel);
    return 0;
}

/* ========================================================================== */
/* Control                                                                     */
/* ========================================================================== */

int Profet::enable()
{
    if (!initialized_)
    {
        LOG_ERR("[%s] not initialized", name_);
        return -1;
    }

    int ret = gate_->set(true);
    if (ret != 0)
    {
        LOG_ERR("[%s] gate assert failed: %d", name_, ret);
        return ret;
    }
    enabled_ = true;
    LOG_DBG("[%s] enabled", name_);
    return 0;
}

int Profet::disable()
{
    if (!initialized_)
    {
        LOG_ERR("[%s] not initialized", name_);
        return -1;
    }

    int ret = gate_->set(false);
    if (ret != 0)
    {
        LOG_ERR("[%s] gate deassert failed: %d", name_, ret);
        return ret;
    }
    enabled_ = false;
    LOG_DBG("[%s] disabled", name_);
    return 0;
}

int Profet::toggle()
{
    return enabled_ ? disable() : enable();
}

/* ========================================================================== */
/* Current sensing                                                             */
/* ========================================================================== */

int16_t Profet::read_current_raw()
{
    switch (sense_mode_)
    {
    case SenseMode::DIRECT:
        return direct_adc_->read_raw();

    case SenseMode::MUXED:
        return mux_->read_raw(mux_channel_);

    default:
        return -1;
    }
}

float Profet::read_current_amps()
{
    if (sense_mode_ == SenseMode::NONE || k_ilis_ <= 0.0f)
    {
        return -1.0f;
    }

    float v_is = 0.0f;

    switch (sense_mode_)
    {
    case SenseMode::DIRECT:
        v_is = direct_adc_->read_voltage();
        break;

    case SenseMode::MUXED:
        v_is = mux_->read_voltage(mux_channel_);
        break;

    default:
        return -1.0f;
    }

    // I_LOAD = (V_IS / R_SENSE) * kILIS
    float i_load = (v_is / r_sense_) * k_ilis_;
    LOG_DBG("[%s] V_IS=%.4f V  I_LOAD=%.3f A", name_, (double)v_is, (double)i_load);
    return i_load;
}