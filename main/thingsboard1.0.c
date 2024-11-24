#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_random.h"

#define SSID "Redmi"
#define PASS "minh2603"
#define THINGSBOARD_SERVER "mqtt://thingsboard.cloud"
#define THINGSBOARD_PORT 1883
#define THINGSBOARD_TOKEN "kFLmaH8ENYJWskCiFMIn"
#define THINGSBOARD_TOPIC "v1/devices/me/telemetry"

static const char *TAG = "mqtt_thingsboard"; 
esp_mqtt_client_handle_t client;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR "\n", IP2STR(&event->ip_info.ip));
    }
}

void wifi_init_sta(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}
void mqtt_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

while (1) {
                int random_number = esp_random() % 100; 
                char msg[100];
                snprintf(msg, sizeof(msg), "{\"random_number\": %d}", random_number);
                esp_mqtt_client_publish(client, THINGSBOARD_TOPIC, msg, 0, 1, 0);
                vTaskDelay(1000 / portTICK_PERIOD_MS); 
            }

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
}

void app_main(void) {
    nvs_flash_init();
    wifi_init_sta();

    esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init(); }
            ESP_ERROR_CHECK(ret);
            esp_mqtt_client_config_t mqtt_cfg = {
                .broker.address.uri = THINGSBOARD_SERVER,
                .credentials.username = THINGSBOARD_TOKEN,
            };
            client =esp_mqtt_client_init(&mqtt_cfg);
            esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);


       esp_mqtt_client_start(client); }