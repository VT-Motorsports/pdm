#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "hardware.h"
#include "system.h"
#include "vehicle_state.h"

LOG_MODULE_REGISTER(main);

int main(void)
{
    LOG_INF("***PDM ENTERED MAIN***");

    static VehicleState vehicle;
    static Hardware hardware(&vehicle);
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

    // Start the system diagnostics task (1000 ms period, priority 10).
    start_diagnostics_task(&system, &hardware, &vehicle);

    if (hardware.power_mgr.enable_all() != 0)
    {
        LOG_ERR("Failed to enable all profets");
        return -3;
    };

    LOG_INF("=== PDM Ready, all PROFETs ON ===");

    while (1)
    {
        k_sleep(K_FOREVER);
    }
}
