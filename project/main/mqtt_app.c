#include "mqtt_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "mqtt_client.h"
#include <string.h>

static const char *TAG = "MQTT_APP";
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_published = false;

// Wi-Fi credentials
#define WIFI_SSID       "VilarSpot"
#define WIFI_PASS       "tondelacity"

// Event handler for Wi-Fi events
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        // Don't auto-reconnect - we want fast failure
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool wifi_init_and_connect(void)
{
    ESP_LOGI(TAG, "Initializing Wi-Fi...");
    
    // Initialize TCP stack and event loop if not already done
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create event group for synchronization
    wifi_event_group = xEventGroupCreate();
    
    // Initialize Wi-Fi
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    // Configure Wi-Fi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold = {
                .authmode = WIFI_AUTH_WPA2_PSK,  // Minimum security
            },
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Wait for connection with timeout
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdTRUE, 
                                           pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Wi-Fi connected successfully");
        return true;
    } else {
        ESP_LOGW(TAG, "Wi-Fi connection timeout (%d ms)", WIFI_CONNECT_TIMEOUT_MS);
        // Clean up event handler but don't deinit everything (will be cleaned on next wake)
        esp_wifi_stop();
        return false;
    }
}

// MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected to broker");
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            break;
            
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Message published successfully, msg_id=%d", event->msg_id);
            mqtt_published = true;
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;
            
        default:
            break;
    }
}

bool mqtt_init(void)
{
    ESP_LOGI(TAG, "Initializing MQTT with TLS...");
    
    // MQTT configuration with TLS
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTT_BROKER_URI,
            },
            .verification = {
                .skip_cert_common_name_check = false,  // Strict verification
            }
        },
        .credentials = {
            .username = "aseadmin",  
            .authentication = {
                .password = "ASEadmin123",  
                .certificate = NULL,
                .key = NULL,
            }
        },
        .network = {
            .timeout_ms = 3000,  // 3 second timeout
        },
        .session = {
            .keepalive = 10,  // 10 seconds keepalive
        }
    };
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        return false;
    }
    
    // Wait briefly for connection (non-blocking, just a quick check)
    vTaskDelay(pdMS_TO_TICKS(500));
    return true;
}

bool mqtt_send_alert(void)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return false;
    }
    
    mqtt_published = false;
    
    ESP_LOGI(TAG, "Sending alert: %s", MQTT_ALERT_MESSAGE);
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_ALERT, 
                                         MQTT_ALERT_MESSAGE, 0, 1, 0);  // QoS 1
    
    if (msg_id <= 0) {
        ESP_LOGE(TAG, "Failed to publish message");
        return false;
    }
    
    // Wait for publish confirmation with timeout
    int attempts = 0;
    while (!mqtt_published && attempts < (MQTT_CONNECT_TIMEOUT_MS / 100)) {
        vTaskDelay(pdMS_TO_TICKS(100));
        attempts++;
    }
    
    return mqtt_published;
}

void mqtt_cleanup(void)
{
    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
    
    // Disconnect Wi-Fi cleanly
    esp_wifi_disconnect();
    esp_wifi_stop();
    
    // Unregister event handlers
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler);
    
    if (wifi_event_group) {
        vEventGroupDelete(wifi_event_group);
        wifi_event_group = NULL;
    }
    
    ESP_LOGI(TAG, "MQTT and Wi-Fi cleaned up");
}

bool is_wifi_connected(void)
{
    if (!wifi_event_group) return false;
    EventBits_t bits = xEventGroupGetBits(wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}