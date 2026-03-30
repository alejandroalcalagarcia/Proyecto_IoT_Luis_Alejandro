#include "mqtt_beebotte.h"
#include "lvgl_beebotte_ui.h" // ¡Incluimos la librería de la pantalla!
#include "esp_log.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "MQTT_BEEBOTTE";
static const char *SUB_TOPIC = "test/temp_ext"; 

// Función para extraer solo el valor del JSON de Beebotte
static void extract_data_field(const char *payload, int payload_len, char *out_value, size_t out_value_size) {
    if (payload == NULL || out_value == NULL || out_value_size == 0) return;
    out_value[0] = '\0';
    char local_payload[256];
    size_t copy_len = (payload_len < (int)(sizeof(local_payload) - 1)) ? (size_t)payload_len : (sizeof(local_payload) - 1);
    memcpy(local_payload, payload, copy_len);
    local_payload[copy_len] = '\0';

    const char *data_key = strstr(local_payload, "\"data\"");
    if (data_key == NULL) { snprintf(out_value, out_value_size, "%s", local_payload); return; }
    const char *colon = strchr(data_key, ':');
    if (colon == NULL) { snprintf(out_value, out_value_size, "%s", local_payload); return; }
    colon++;
    while (*colon == ' ' || *colon == '\t') colon++;

    if (*colon == '"') {
        colon++;
        const char *end_quote = strchr(colon, '"');
        if (end_quote == NULL) { snprintf(out_value, out_value_size, "%s", local_payload); return; }
        size_t value_len = (size_t)(end_quote - colon);
        if (value_len >= out_value_size) value_len = out_value_size - 1;
        memcpy(out_value, colon, value_len);
        out_value[value_len] = '\0';
        return;
    }

    const char *end_value = colon;
    while (*end_value != '\0' && *end_value != ',' && *end_value != '}') end_value++;
    size_t value_len = (size_t)(end_value - colon);
    while (value_len > 0 && (colon[value_len - 1] == ' ' || colon[value_len - 1] == '\t')) value_len--;
    if (value_len >= out_value_size) value_len = out_value_size - 1;
    memcpy(out_value, colon, value_len);
    out_value[value_len] = '\0';
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "¡Conectado exitosamente a Beebotte!");
            esp_mqtt_client_subscribe(event->client, SUB_TOPIC, 0); 
            ESP_LOGI(TAG, "Suscrito al topic: %s", SUB_TOPIC);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado del broker MQTT");
            break;
        case MQTT_EVENT_DATA: {
            char payload[256];
            char parsed_value[64];

            int data_len = (event->data_len < (int)(sizeof(payload) - 1)) ? event->data_len : (int)(sizeof(payload) - 1);
            memcpy(payload, event->data, data_len);
            payload[data_len] = '\0';

            // Extraemos el número y lo mandamos a la pantalla
            extract_data_field(payload, data_len, parsed_value, sizeof(parsed_value));
            lvgl_beebotte_ui_update_value(parsed_value);
            
            ESP_LOGI(TAG, "Actualizando pantalla con valor: %s", parsed_value);
            break;
        }
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
            .username = "K5f4wRShKh2Q3ojjfOOFG1qW", 
            .authentication = {
                .password = "pw3uxhADNjKscUU16ZpsYYbJR73CSnvT", 
            },
            .client_id = "esp32_alejandro_interior", 
        },
        .network = {
            .timeout_ms = 10000,
        }
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    return client;
}