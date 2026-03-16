#include "mqtt_beebotte.h"
#include "esp_log.h"
#include <inttypes.h>

static const char *TAG = "MQTT_BEEBOTTE";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "¡Conectado exitosamente a Beebotte!");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado del broker MQTT");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "¡Confirmado por el Broker! msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Error en MQTT");
            break;
        default:
            break;
    }
}

esp_mqtt_client_handle_t mqtt_beebotte_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtt.beebotte.com",
        .credentials = {
            .username = "iamtkn_D6UYwC2AekJFYTd9", // Token de Beebotte
            .client_id = "esp32_alejandro_interior", // ID único de tu placa
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    
    return client;
}