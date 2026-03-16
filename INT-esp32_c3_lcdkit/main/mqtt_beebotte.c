#include "mqtt_beebotte.h"
#include "esp_log.h"

static const char *TAG = "MQTT_BEEBOTTE";

// Callback para manejar eventos MQTT (recepción de mensajes)
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado a Beebotte.");
            // ¡Nos suscribimos al topic del compañero!
            esp_mqtt_client_subscribe(client, "test/res", 0);
            ESP_LOGI(TAG, "Suscrito al topic: test/res");
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado del broker");
            break;
            
        case MQTT_EVENT_DATA:
            // Este evento salta cuando el compañero (o tú mismo) publica algo
            ESP_LOGI(TAG, "=================================");
            ESP_LOGI(TAG, "MENSAJE RECIBIDO (Suscripción)");
            printf("Topic: %.*s\r\n", event->topic_len, event->topic);
            printf("Data: %.*s\r\n", event->data_len, event->data);
            ESP_LOGI(TAG, "=================================");
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
            // Usa las credenciales de tu compañero para conectar al mismo canal
            .username = "K5f4wRShKh2Q3ojjfOOFG1qW",    
            .authentication = {
                .password = "pw3uxhADNjKscUU16ZpsYYbJR73CSnvT", 
            },
            .client_id = "esp32_c3_recolector_interior" // ID Único diferente al de tu compañero
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    
    return client;
}
