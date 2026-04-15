// pwm_controller.hpp
#pragma once

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

#define PWM_PERIOD_TIM1_NS 2000000U // 500Hz  - rad2, pump1, pump2
#define PWM_PERIOD_TIM2_NS 40000U   // 25kHz  - fan1, fan2
#define PWM_PERIOD_TIM3_NS 40000U   // 25kHz  - fan3, rad1, fan4

struct PwmOutput
{
    const struct pwm_dt_spec spec;
    const char *name;
    uint32_t period_ns;
    uint32_t duty_ns;
};

class PwmController
{
  public:
    PwmController();

    // Set all PWM outputs to the same duty cycle (0-100)
    int setAll(uint8_t duty_percent);

    // Set individual PWM outputs (0-100)
    int setRad1(uint8_t duty_percent);
    int setRad2(uint8_t duty_percent);
    int setPump1(uint8_t duty_percent);
    int setPump2(uint8_t duty_percent);
    int setFan1(uint8_t duty_percent);
    int setFan2(uint8_t duty_percent);
    int setFan3(uint8_t duty_percent);
    int setFan4(uint8_t duty_percent);

    // Thread entry
    void run();

    static void threadEntry(void *p1, void *p2, void *p3)
    {
        static_cast<PwmController *>(p1)->run();
    }

  private:
    int applyDuty(PwmOutput &output, uint8_t duty_percent);
    int setChannel(size_t index, uint8_t duty_percent);

    // PWM outputs indexed for easy iteration
    enum Channel
    {
        RAD2 = 0,
        PUMP1,
        PUMP2,
        FAN1,
        FAN2,
        FAN3,
        RAD1,
        FAN4,
        CHANNEL_COUNT
    };

    PwmOutput outputs[CHANNEL_COUNT];
    uint8_t global_duty;
};

PwmController *startPwmController(uint8_t initial_duty);