#include "can_manager.h"

#if KEYA_MOTOR

#include "config/pinout.h"
#include "utils/log.h"

namespace hw {
namespace can {

static bool driverInstalled = false;

bool init() {
    if (driverInstalled) return true;

    pinMode(CAN_RX_PIN, INPUT);
    pinMode(CAN_TX_PIN, OUTPUT);

#if KEYA_NO_ACK
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NO_ACK);
    info("CAN: NO_ACK mode (bench/sniffer build)");
#else
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
#endif
    g_config.alerts_enabled = TWAI_ALERT_ABOVE_ERR_WARN
                            | TWAI_ALERT_BUS_ERROR
                            | TWAI_ALERT_TX_FAILED
                            | TWAI_ALERT_RX_QUEUE_FULL
                            | TWAI_ALERT_BUS_OFF;
    g_config.rx_queue_len = 32;
    g_config.tx_queue_len = 16;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        error("CAN: driver install failed");
        return false;
    }
    if (twai_start() != ESP_OK) {
        error("CAN: start failed");
        twai_driver_uninstall();
        return false;
    }

    driverInstalled = true;
    infof("CAN: started at 250kbps (TX=%d RX=%d)", CAN_TX_PIN, CAN_RX_PIN);
    return true;
}

bool send(const twai_message_t& msg, TickType_t timeout) {
    if (!driverInstalled) return false;
    return twai_transmit(&msg, timeout) == ESP_OK;
}

bool receive(twai_message_t& msg, TickType_t timeout) {
    if (!driverInstalled) return false;
    return twai_receive(&msg, timeout) == ESP_OK;
}

static void drainAlerts() {
    uint32_t alerts = 0;
    if (twai_read_alerts(&alerts, 0) != ESP_OK || alerts == 0) return;
    if (alerts & TWAI_ALERT_BUS_OFF)        error("CAN: bus-off");
    if (alerts & TWAI_ALERT_BUS_ERROR)      error("CAN: bus error");
    if (alerts & TWAI_ALERT_TX_FAILED)      error("CAN: tx failed");
    if (alerts & TWAI_ALERT_RX_QUEUE_FULL)  warning("CAN: rx queue full");
    if (alerts & TWAI_ALERT_ABOVE_ERR_WARN) warning("CAN: error counter above warn");
}

[[noreturn]] void task(void* pvParameters) {
    (void)pvParameters;
    while (true) {
        drainAlerts();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

} // namespace can
} // namespace hw

#endif // KEYA_MOTOR
