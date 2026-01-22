#include "zephyr-common.h"
#include "globals.h"
#include "zephyr/drivers/can.h"
#include "zephyr/kernel.h"

LOG_MODULE_REGISTER(canInitializer, LOG_LEVEL_INF);


namespace {
    constexpr int CAN1_BAUD = 1000000;         
    constexpr int CAN1_SAMPLE_POINT = 875;       
    constexpr uint16_t CAN1_STATUS_MSG_ID = 0x090; 
}

const struct device *can1; 
CAN_MSGQ_DEFINE(can1_rx_test_msgq, 10);


static void can_status_callback(const struct device *dev, int error, void *user_data) {
    if (error) {
        LOG_ERR("CAN Status MSG Transmit failed: %d", error);
    } else {
        LOG_INF("CAN Status MSG Transmit succeeded");
    }
}
uint8_t can_init() {

    can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));


    if (!device_is_ready(can1)) {
        LOG_ERR("CAN1 controller not ready");
        return static_cast<uint8_t>(-1);
    }

    LOG_INF("CAN1 device ready — calculating timing...");
    struct can_timing timing {};
    int ret = can_calc_timing(can1, &timing, CAN1_BAUD, CAN1_SAMPLE_POINT);
    if (ret != 0) {
        LOG_ERR("can_calc_timing() failed with code %d", ret);
        return static_cast<uint8_t>(ret);
    }

    struct can_bus_err_cnt can1err; 
    enum can_state state;
    can_get_state(can1, &state,&can1err);
    if (state != CAN_STATE_STOPPED) {
        LOG_INF("Stopping CAN1 to Configure Timing!");
        can_stop(can1);
        if (ret != 0) {
        LOG_ERR("can_stop() failed with code %d", ret);
        return static_cast<uint8_t>(ret);
    }
    }

    LOG_INF("Applying timing configuration...");
    ret = can_set_timing(can1, &timing);
    if (ret != 0) {
        LOG_ERR("can_set_timing() failed with code %d", ret);
        return static_cast<uint8_t>(ret);
    }


    // LOG_INF("Setting LOOPBACK MODE"); 
    // ret = can_set_mode(can1, CAN_MODE_LOOPBACK);
    // if (ret) {
    //     LOG_ERR("Failed to set CAN loopback mode: %d", ret);
    // }
    // else{
    //     LOG_INF("Loopback Set"); 
    // }


    LOG_INF("CAN mode flags: 0x%x", can_get_mode(can1));

    LOG_INF("Starting CAN1...");
    ret = can_start(can1);
    if (ret != 0) {
        LOG_ERR("can_start() failed with code %d", ret);
        return static_cast<uint8_t>(ret);
    }

    struct can_frame msg_can1_status{
        .id = CAN1_STATUS_MSG_ID, 
        .dlc = can_bytes_to_dlc(8), 
        .flags = 0, 
        .data = {0,1,2,3,4,5,6,7}
    }; 

    ret = can_send(can1, &msg_can1_status, K_MSEC(10), can_status_callback, nullptr);
    if (ret < 0) {
        LOG_ERR("Failed to send CAN frame: %d", ret);
    }
    LOG_INF("CAN1 initialization complete.");

	const struct can_filter match_all_rx = {
        .id = 0x000,
        .mask = ~CAN_STD_ID_MASK,
		.flags = 0,
	};
    int test_msgq_id = can_add_rx_filter_msgq(can1, &can1_rx_test_msgq, &match_all_rx);
    
    if (test_msgq_id < 0) {
        LOG_ERR("Failed to add RX MSGQ: %d", test_msgq_id);
    } else {
        LOG_INF("RX MSGQ installed successfully (id=%d)", test_msgq_id);
    }

    LOG_INF("Waiting to Recieve first Message"); 

    struct can_frame rx_frame;
    ret = k_msgq_get(&can1_rx_test_msgq, &rx_frame,K_FOREVER);
    if (ret == 0) {
        LOG_INF("Received first CAN frame:");
        LOG_INF("  ID: 0x%x", rx_frame.id);
        LOG_INF("  DLC: %d", rx_frame.dlc);
        LOG_HEXDUMP_INF(rx_frame.data, can_dlc_to_bytes(rx_frame.dlc), "  Data:");
    } else {
        LOG_ERR("Failed to receive CAN frame (err=%d)", ret);
    }


    can_remove_rx_filter(can1,test_msgq_id); 
    k_msgq_purge(&can1_rx_test_msgq); 
    LOG_INF("Test MSGQ Destroyed, Filter Removed. CAN1 IS READY"); 

    return 0;
}
