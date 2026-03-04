// mux_adc.cpp
#include "mux_adc.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mux_adc);

/* ========================================================================== */

int MuxAdcChannel::init(GpioPin *s0, GpioPin *s1, AdcChannel *adc, const char *name)
{
    if (!s0 || !s1 || !adc)
    {
        LOG_ERR("[mux %s] NULL argument passed to init", name);
        return -1;
    }

    s0_   = s0;
    s1_   = s1;
    adc_  = adc;
    name_ = name;

    /* Park select lines at channel 0 on startup */
    s0_->set(false);
    s1_->set(false);

    initialized_ = true;
    LOG_INF("[mux %s] initialized", name_);
    return 0;
}

/* -------------------------------------------------------------------------- */

void MuxAdcChannel::select(uint8_t channel)
{
    /* channel is 2 bits: bit0 → S0, bit1 → S1 */
    s0_->set(channel & 0x01U);
    s1_->set((channel >> 1U) & 0x01U);

    /* Wait for signal to settle through mux + any RC filter */
    k_busy_wait(SETTLE_US);
}

/* -------------------------------------------------------------------------- */

int16_t MuxAdcChannel::read_raw(uint8_t channel)
{
    if (!initialized_)
    {
        LOG_ERR("[mux %s] not initialized", name_);
        return -1;
    }
    if (channel > 3U)
    {
        LOG_ERR("[mux %s] channel %u out of range (0-3)", name_, channel);
        return -1;
    }

    select(channel);
    return adc_->read_raw();
}

/* -------------------------------------------------------------------------- */

float MuxAdcChannel::read_voltage(uint8_t channel)
{
    if (!initialized_)
    {
        LOG_ERR("[mux %s] not initialized", name_);
        return -1.0f;
    }
    if (channel > 3U)
    {
        LOG_ERR("[mux %s] channel %u out of range (0-3)", name_, channel);
        return -1.0f;
    }

    select(channel);
    return adc_->read_voltage();
}
