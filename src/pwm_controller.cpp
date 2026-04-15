// pwm_controller.cpp
#include "pwm_controller.h"

LOG_MODULE_REGISTER(pwm_controller, LOG_LEVEL_DBG);

#define STACK_SIZE 1024
#define PRIORITY 5
#define UPDATE_INTERVAL_MS 100

K_THREAD_STACK_DEFINE(pwm_stack, STACK_SIZE);
static struct k_thread pwm_thread_data;

PwmController::PwmController()
    : global_duty(0), outputs{
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_rad2)), "rad2", PWM_PERIOD_TIM1_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_pump1)), "pump1", PWM_PERIOD_TIM1_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_pump2)), "pump2", PWM_PERIOD_TIM1_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_fan1)), "fan1", PWM_PERIOD_TIM2_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_fan2)), "fan2", PWM_PERIOD_TIM2_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_fan3)), "fan3", PWM_PERIOD_TIM3_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_rad1)), "rad1", PWM_PERIOD_TIM3_NS, 0},
                          {PWM_DT_SPEC_GET(DT_ALIAS(pwm_fan4)), "fan4", PWM_PERIOD_TIM3_NS, 0},
                      }
{
}

int PwmController::applyDuty(PwmOutput &output, uint8_t duty_percent)
{
    if (duty_percent > 100)
        duty_percent = 100;

    uint32_t pulse_ns = (output.period_ns / 100) * duty_percent;
    output.duty_ns = pulse_ns;

    int ret = pwm_set_dt(&output.spec, output.period_ns, pulse_ns);
    if (ret != 0)
    {
        LOG_ERR("Failed to set PWM %s to %d%%: %d", output.name, duty_percent, ret);
    }
    else
    {
        LOG_DBG("PWM %s set to %d%% (pulse: %u ns)", output.name, duty_percent, pulse_ns);
    }

    return ret;
}

int PwmController::setChannel(size_t index, uint8_t duty_percent)
{
    if (index >= CHANNEL_COUNT)
    {
        return -EINVAL;
    }
    return applyDuty(outputs[index], duty_percent);
}

int PwmController::setAll(uint8_t duty_percent)
{
    global_duty = duty_percent;
    int ret = 0;
    for (size_t i = 0; i < CHANNEL_COUNT; i++)
    {
        int r = applyDuty(outputs[i], duty_percent);
        if (r != 0)
            ret = r;
    }
    return ret;
}

int PwmController::setRad1(uint8_t duty_percent)
{
    return setChannel(RAD1, duty_percent);
}
int PwmController::setRad2(uint8_t duty_percent)
{
    return setChannel(RAD2, duty_percent);
}
int PwmController::setPump1(uint8_t duty_percent)
{
    return setChannel(PUMP1, duty_percent);
}
int PwmController::setPump2(uint8_t duty_percent)
{
    return setChannel(PUMP2, duty_percent);
}
int PwmController::setFan1(uint8_t duty_percent)
{
    return setChannel(FAN1, duty_percent);
}
int PwmController::setFan2(uint8_t duty_percent)
{
    return setChannel(FAN2, duty_percent);
}
int PwmController::setFan3(uint8_t duty_percent)
{
    return setChannel(FAN3, duty_percent);
}
int PwmController::setFan4(uint8_t duty_percent)
{
    return setChannel(FAN4, duty_percent);
}

void PwmController::run()
{
    LOG_INF("PWM controller thread started");

    // Verify all devices are ready
    for (size_t i = 0; i < CHANNEL_COUNT; i++)
    {
        if (!pwm_is_ready_dt(&outputs[i].spec))
        {
            LOG_ERR("PWM device %s not ready", outputs[i].name);
            return;
        }
    }

    // Initial duty cycle
    setAll(global_duty);

    while (true)
    {
        // Main loop — extend this with control logic later
        k_sleep(K_MSEC(UPDATE_INTERVAL_MS));
    }
}

// Factory to create and start the thread
PwmController *startPwmController(uint8_t initial_duty)
{
    static PwmController controller;
    controller.setAll(initial_duty); // set before thread starts

    k_thread_create(&pwm_thread_data, pwm_stack, STACK_SIZE, PwmController::threadEntry, &controller, nullptr, nullptr,
                    PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(&pwm_thread_data, "pwm_ctrl");
    return &controller;
}