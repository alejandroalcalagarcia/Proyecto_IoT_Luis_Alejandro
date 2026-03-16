#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h" // Para el WiFi (example_connect)

#include "sensor_temperatura.h"
#include "mqtt_beebotte.h"

static const char *TAG = "MAIN_RECOLECTOR";

void app_main(void) {
    // 1. Inicialización de NVS, Red y Event Loop (Obligatorio para WiFi y MQTT)
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 2. Conectar al WiFi (configurado por menuconfig)
    ESP_LOGI(TAG, "Conectando a WiFi...");
    ESP_ERROR_CHECK(example_connect());

    // 3. Inicializar MQTT (Suscriptor)
    ESP_LOGI(TAG, "Iniciando cliente MQTT...");
    esp_mqtt_client_handle_t mqtt_client = mqtt_beebotte_init();
    vTaskDelay(pdMS_TO_TICKS(2000)); // Esperar conexión

    // 4. Inicializar Sensor de Temperatura Local
    float temp_local = 0;
    float hum_local  = 0;
    if (sensor_temperature_init() != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al inicializar sensor AHT10");
    }

    // 5. Bucle principal
    while (1) {
        // Leemos nuestra propia planta (Interior)
        if (sensor_temperature_read(&temp_local, &hum_local) == ESP_OK) {
            printf("-> Temp Interior Local: %.2f °C | Humedad: %.2f %%\n", temp_local, hum_local);

            // Opcional: Podrías publicar también tu temperatura a otro topic (ej: "test/interior")
            // char payload[64];
            // sprintf(payload, "{\"data\":%.2f,\"write\":true}", temp_local);
            // esp_mqtt_client_publish(mqtt_client, "test/interior", payload, 0, 0, 0);

        } else {
            ESP_LOGE(TAG, "Error leyendo sensor local");
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
