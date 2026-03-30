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
#include "lvgl_beebotte_ui.h" // Incluimos tu archivo de UI
#include "bsp/esp-bsp.h"      // Incluimos el controlador físico de la pantalla

static const char *TAG = "MAIN_RECOLECTOR";

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 1. PRIMERO: Conectamos a Wi-Fi
    ESP_LOGI(TAG, "Conectando a WiFi...");
    ESP_ERROR_CHECK(example_connect());

    // 2. SEGUNDO: Arrancamos MQTT y le damos tiempo de sobra para conectarse
    ESP_LOGI(TAG, "Iniciando cliente MQTT...");
    esp_mqtt_client_handle_t mqtt_client = mqtt_beebotte_init();
    
    // Truco 2: Dejamos que el Wi-Fi "respire" y negocie con Beebotte durante 4 segundos
    vTaskDelay(pdMS_TO_TICKS(4000)); 

    // 3. TERCERO: Ahora que todo es estable, encendemos la pantalla
    ESP_LOGI(TAG, "Inicializando la pantalla LCD...");
    bsp_display_start();        
    // Truco 3: Bajamos el brillo al 30% para que el cable USB no colapse de energía
    bsp_display_brightness_set(50); 

    if (lvgl_beebotte_ui_init() != ESP_OK) {
        ESP_LOGW(TAG, "LVGL UI no inicializada.");
    }

    float temp_local = 0;
    float hum_local = 0;
    
    if (sensor_temperature_init() != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al inicializar sensor AHT10");
    }

    char payload[64];
    const char *topic_temp = "test/temp_int"; 

    // BUCLE PRINCIPAL
    // BUCLE PRINCIPAL
    while (1) {
        if (sensor_temperature_read(&temp_local, &hum_local) == ESP_OK) {
            
            printf("-> Temp Local: %.2f °C | Hum Local: %.2f %%\n", temp_local, hum_local);
            
            // ¡NUEVA LÍNEA! Mandamos la temperatura interior a la pantalla
            lvgl_beebotte_ui_update_interior(temp_local);

            sprintf(payload, "{\"data\":%.2f,\"write\":true}", temp_local);
            int msg_id = esp_mqtt_client_publish(mqtt_client, topic_temp, payload, 0, 1, 0);
            
            if (msg_id != -1) {
                ESP_LOGI(TAG, "Publicado en Beebotte: %.2f ºC", temp_local);
            }
            
        } else {
            ESP_LOGE(TAG, "Error leyendo sensor local");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); 
    }
}