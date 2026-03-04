#pragma once

// mux_adc.h
//
// Models a 2-bit analog multiplexer feeding a single ADC input.
//
// Select line truth table (S1 = MSB, S0 = LSB):
//   S1=0, S0=0  →  channel 0  (binary 00)
//   S1=0, S0=1  →  channel 1  (binary 01)
//   S1=1, S0=0  →  channel 2  (binary 10)
//   S1=1, S0=1  →  channel 3  (binary 11)
//
// A brief settling delay is inserted after the select lines change before
// the ADC is sampled.  Tune SETTLE_US for your mux + RC filter combination.

#include "adc.h"
#include "gpio.h"
#include <cstdint>

class MuxAdcChannel
{
  public:
    /// Settling time in microseconds after changing select lines.
    static constexpr uint32_t SETTLE_US = 10U;

    MuxAdcChannel() = default;

    /// @param s0     Select bit-0 (LSB) GPIO pin
    /// @param s1     Select bit-1 (MSB) GPIO pin
    /// @param adc    ADC channel connected to the mux output
    /// @param name   Label used in log messages
    int init(GpioPin *s0, GpioPin *s1, AdcChannel *adc, const char *name);

    /// Select mux input [0..3], wait for settling, then return raw ADC count.
    int16_t read_raw(uint8_t channel);

    /// Select mux input [0..3], wait for settling, then return voltage in V.
    float read_voltage(uint8_t channel);

    const char *name() const
    {
        return name_;
    }
    bool is_ready() const
    {
        return initialized_;
    }

  private:
    GpioPin *s0_{nullptr};
    GpioPin *s1_{nullptr};
    AdcChannel *adc_{nullptr};
    const char *name_{"?"};
    bool initialized_{false};

    /// Drive select lines to address [0..3].
    void select(uint8_t channel);
};
