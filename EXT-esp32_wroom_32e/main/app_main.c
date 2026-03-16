/* MQTT (over TCP) Example */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h" // Necesario para vTaskDelay
#include "freertos/task.h"     // Necesario para vTaskDelay

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "mqtt_beebotte";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/** * @brief Event handler registered to receive MQTT events
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "¡Conectado exitosamente a Beebotte!");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Desconectado del broker MQTT");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
        }
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "¡Confirmado por el Broker! msg_id=%d", event->msg_id);
        break;
    default:
        ESP_LOGI(TAG, "Otro evento id:%d", event->event_id);
        break;
    }
}

/*
// Modificamos esta función para que devuelva el cliente MQTT
static esp_mqtt_client_handle_t mqtt_app_start(void)
{
// Usamos designadores de campos para evitar errores de versión de IDF
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtt.beebotte.com",
        .credentials = {
            .username = "iamtkn_D6UYwC2AekJFYTd9", // Prefijo incluido
            .client_id = "esp32_luis_unique_123",        // ID único
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    
    return client;
}
*/




static esp_mqtt_client_handle_t mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtt.beebotte.com",
        .credentials = {
            .username = "K5f4wRShKh2Q3ojjfOOFG1qW",    // La que empieza por 'ak_...'
            .authentication = {
                .password = "pw3uxhADNjKscUU16ZpsYYbJR73CSnvT", // La que empieza por 'sk_...'
            },
            .client_id = "esp32_luis_test"
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    
    return client;
}
void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Se conecta al WiFi configurado en menuconfig
    ESP_ERROR_CHECK(example_connect());

    // Iniciamos MQTT y guardamos el cliente
    esp_mqtt_client_handle_t client = mqtt_app_start();

    // Damos un par de segundos para que se establezca la conexión MQTT antes de publicar
    vTaskDelay(pdMS_TO_TICKS(2000));

    // --- BUCLE PRINCIPAL DE TEMPERATURA FALSA ---
    int temperatura = 0;
    char payload[64];
    
    // El tópico de Beebotte suele ser: Canal/Recurso
    const char *topic = "test/res"; 

    while (1) {
        // Formateamos el entero como texto para enviarlo
        sprintf(payload, "{\"data\":%d,\"write\":true}", temperatura);
        
        // Publicamos (QoS 0)
        int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
        
        if(msg_id != -1){
            ESP_LOGI(TAG, "Publicado: %d ºC al tópico %s", temperatura, topic);
        } else {
            ESP_LOGE(TAG, "Error al publicar. Revisa la conexión.");
        }

        temperatura++; // Incrementamos
        
        // Esperamos 1000 milisegundos (1 segundo)
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}