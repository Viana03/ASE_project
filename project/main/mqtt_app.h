#ifndef MQTT_APP_H
#define MQTT_APP_H

#include <stdbool.h>

// MQTT Broker configuration
#define MQTT_BROKER_URI     "mqtts://5b0a738de424409d8a12ca674057856d.s1.eu.hivemq.cloud:8883"
#define MQTT_TOPIC_ALERT    "ase/bike/alert"
#define MQTT_ALERT_MESSAGE  "{\"type\":\"movement\",\"status\":\"suspicious_motion_detected\"}"

// Timeouts (milliseconds)
#define WIFI_CONNECT_TIMEOUT_MS    5000
#define MQTT_CONNECT_TIMEOUT_MS    3000

/**
 * @brief Initialize Wi-Fi in station mode and connect to configured network
 * @return true if connected successfully within timeout, false otherwise
 */
bool wifi_init_and_connect(void);

/**
 * @brief Initialize MQTT client with TLS support
 * @return true if MQTT client initialized successfully, false otherwise
 */
bool mqtt_init(void);

/**
 * @brief Send alert message via MQTT (blocking with timeout)
 * @return true if message published successfully, false otherwise
 */
bool mqtt_send_alert(void);

/**
 * @brief Disconnect Wi-Fi and MQTT, clean up resources
 */
void mqtt_cleanup(void);

/**
 * @brief Check if Wi-Fi is currently connected
 */
bool is_wifi_connected(void);

#endif // MQTT_APP_H