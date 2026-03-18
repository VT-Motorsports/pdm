#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "hardware.h"
#include "system.h"
#include "vehicle_state.h"

LOG_MODULE_REGISTER(main);

void send_heartbeat(Hardware &hw)
{
    static uint32_t heartbeat_counter = 0;

    struct can_frame heartbeat_frame = {.id = 0x100, // Heartbeat message ID
                                        .dlc = 8,
                                        .flags = 0,
                                        .data = {(uint8_t)(heartbeat_counter >> 24), (uint8_t)(heartbeat_counter >> 16),
                                                 (uint8_t)(heartbeat_counter >> 8), (uint8_t)(heartbeat_counter), 0xEF,
                                                 0xFF, 0xFF, 0xFF}};

    // int ret = hw.can1.send(&heartbeat_frame, K_NO_WAIT);
    // if (ret == 0)
    // {
    // }
    // else
    // {
    //     LOG_ERR("Heartbeat send failed: %d", ret);
    // }

    heartbeat_counter++;
}

// int main(void)
// {
//     LOG_INF("REACHED MAIN"); /* add this first */
//     k_busy_wait(100000);
//     LOG_INF("AFTER WAIT");
//     LOG_INF("***VCU ENTERED MAIN***\n");

//     static VehicleState vehicle;
//     static Hardware hardware(vehicle);
//     static System system;

//     LOG_INF("=== VCU Starting ===");

//     // Initialize system resources
//     if (system.init() != 0)
//     {
//         LOG_ERR("System init failed!");
//         return -1;
//     }

//     // Initialize hardware
//     if (hardware.init() != 0)
//     {
//         LOG_ERR("Hardware init failed!");
//         return -2;
//     }

//     // Start diagnostics monitoring
//     if (system.start_diagnostics(&hardware) != 0)
//     {
//         LOG_ERR("Failed to start diagnostics!");
//         return -3;
//     }

//     LOG_INF("=== VCU Ready ===");

//     while (1)
//     {
//         hardware.led_green.toggle();
//         send_heartbeat(hardware);
//         LOG_INF("LED BLINKING");

//         k_msleep(500);
//     }
// }
int main(void)
{
    LOG_INF("REACHED MAIN"); /* add this first */
    k_busy_wait(100000);
    LOG_INF("AFTER WAIT");

    LOG_INF("***PDM ENTERED MAIN***");

    static VehicleState vehicle;
    static Hardware hardware(vehicle);
    static System system;
    LOG_INF("=== PDM Starting ===");

    if (system.init() != 0)
    {
        LOG_ERR("System init failed!");
        return -1;
    }

    if (hardware.init() != 0)
    {
        LOG_ERR("Hardware init failed!");
        return -2;
    }

    if (hardware.power_mgr.enable_all() != 0)
    {
        LOG_ERR("Failed to enable all profets");
        return -3;
    };

    // Start diagnostics monitoring
    if (system.start_diagnostics(&hardware) != 0)
    {
        LOG_ERR("Failed to start diagnostics!");
        return -3;
    }

    LOG_INF("=== PDM Ready, all PROFETs ON ===");

    bool on = true;

    while (1)
    {
        // if (on)
        // {
        //     hardware.power_mgr.disable_all();
        // }
        // if (!on)
        // {
        //     hardware.power_mgr.enable_all();
        // }
        on = !on;
        hardware.led_green.toggle();
        send_heartbeat(hardware);
        LOG_INF("LED BLINKING");

        k_msleep(500);
    }
}
