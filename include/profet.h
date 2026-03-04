#pragma once

// profet.h
//
// Models a single Infineon PROFET high-side switch channel.
//
// Each PROFET has:
//   - An IN pin  (GpioPin)  : drives the gate HIGH to turn the load on
//   - A sense input, one of:
//       a) Direct AdcChannel   — wired straight to the IS pin
//       b) MuxAdcChannel       — IS pin feeds one of 4 mux inputs;
//                                mux_channel selects which input to read
//
// Current measurement
// -------------------
//   V_IS    = I_IS * R_SENSE
//   I_IS    = I_LOAD / kILIS
//   ∴ I_LOAD = (V_IS / R_SENSE) * kILIS
//
// kILIS is device-specific (check "Sense Current Ratio" in the datasheet).

#include "adc.h"
#include "gpio.h"
#include "mux_adc.h"
#include <cstdint>

class Profet
{
  public:
    /* --------------------------------------------------------------------- */
    /* Construction                                                           */
    /* --------------------------------------------------------------------- */
    Profet() : name_("?"), k_ilis_(0.0f), r_sense_(1200.0f)
    {
    }
    Profet(const char *name, float k_ilis = 0.0f, float r_sense = 1200.0f);

    /* --------------------------------------------------------------------- */
    /* Initialisation — pick ONE of the three overloads                      */
    /* --------------------------------------------------------------------- */

    /// Gate only — no current sensing.
    int init(GpioPin *gate);

    /// Gate + direct AdcChannel wired to the IS/sense output.
    int init(GpioPin *gate, AdcChannel *adc);

    /// Gate + muxed ADC: IS pin feeds mux input [mux_channel] (0-3).
    int init(GpioPin *gate, MuxAdcChannel *mux, uint8_t mux_channel);

    /* --------------------------------------------------------------------- */
    /* Control                                                                */
    /* --------------------------------------------------------------------- */
    int enable();
    int disable();
    int toggle();
    bool is_enabled() const
    {
        return enabled_;
    }

    /* --------------------------------------------------------------------- */
    /* Current sensing                                                        */
    /* --------------------------------------------------------------------- */

    /// Load current in amps, or -1.0f if sensing is not configured.
    float read_current_amps();

    /// Raw ADC count from the IS path, or -1 if unavailable.
    int16_t read_current_raw();

    bool has_current_sense() const
    {
        return sense_mode_ != SenseMode::NONE;
    }

    /* --------------------------------------------------------------------- */
    /* Misc                                                                   */
    /* --------------------------------------------------------------------- */
    const char *name() const
    {
        return name_;
    }

  private:
    enum class SenseMode : uint8_t
    {
        NONE,
        DIRECT,
        MUXED
    };

    const char *name_;
    GpioPin *gate_{nullptr};

    SenseMode sense_mode_{SenseMode::NONE};
    AdcChannel *direct_adc_{nullptr}; /* used when DIRECT */
    MuxAdcChannel *mux_{nullptr};     /* used when MUXED  */
    uint8_t mux_channel_{0};          /* used when MUXED  */

    float k_ilis_;
    float r_sense_;
    bool enabled_{false};
    bool initialized_{false};
};