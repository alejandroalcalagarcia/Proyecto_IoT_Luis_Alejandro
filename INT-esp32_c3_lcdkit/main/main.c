#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h" 

#include "sensor_temperatura.h"
#include "mqtt_beebotte.h"

static const char *TAG = "MAIN_RECOLECTOR";

void app_main(void) {
    // 1. Inicialización base
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 2. Conexión Wi-Fi
    ESP_LOGI(TAG, "Conectando a WiFi...");
    ESP_ERROR_CHECK(example_connect());

    // 3. Inicializar MQTT
    ESP_LOGI(TAG, "Iniciando cliente MQTT...");
    esp_mqtt_client_handle_t mqtt_client = mqtt_beebotte_init();
    vTaskDelay(pdMS_TO_TICKS(2000)); 

    float temp_local = 0;
    float hum_local = 0;
    
    // 4. Inicializar Sensor
    if (sensor_temperature_init() != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al inicializar sensor AHT10");
    }

    char payload[64];
    
    // Tópico de Beebotte: Canal/Recurso
    // IMPORTANTE: Asegúrate de que el canal "test" y el recurso "temp_int" estén creados en Beebotte
    const char *topic_temp = "PROYECTIO_IoT_Alex_Luis/temp_int"; 

    // 5. Bucle Principal
    while (1) {
        if (sensor_temperature_read(&temp_local, &hum_local) == ESP_OK) {
            
            // --> A. Mostrar AMBOS datos por el monitor serie (Terminal) <--
            printf("-> Temp Local: %.2f °C | Hum Local: %.2f %%\n", temp_local, hum_local);
            
            // --> B. Formatear y publicar SOLO la temperatura en Beebotte <--
            // El formato que Beebotte exige es: {"data": valor, "write": true}
            sprintf(payload, "{\"data\":%.2f,\"write\":true}", temp_local);
            
            int msg_id = esp_mqtt_client_publish(mqtt_client, topic_temp, payload, 0, 1, 0);
            
            if (msg_id != -1) {
                ESP_LOGI(TAG, "Publicado en Beebotte: %.2f ºC al topic %s", temp_local, topic_temp);
            } else {
                ESP_LOGE(TAG, "Error al publicar. Revisa la conexión MQTT.");
            }
            
        } else {
            ESP_LOGE(TAG, "Error leyendo sensor local");
        }
        
        // Esperamos 10 segundos antes de la siguiente lectura
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}