#include "lvgl_beebotte_ui.h"
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "LVGL_UI";

// Ahora tenemos DOS textos variables
static lv_obj_t *s_int_label; // Para el sensor interior
static lv_obj_t *s_ext_label; // Para el MQTT exterior
static bool s_ui_ready = false;

// Definimos un "Paquete postal" para saber de dónde viene el dato
typedef enum { MSG_INTERIOR, MSG_EXTERIOR } msg_type_t;

typedef struct {
    msg_type_t type;
    char text[32];
} ui_msg_t;

static QueueHandle_t ui_queue = NULL;

// ---------------------------------------------------------
// TAREA FREERTOS: Pinta la pantalla según el tipo de mensaje
// ---------------------------------------------------------
static void ui_updater_task(void *arg) {
    ui_msg_t rx_msg;
    while (1) {
        // Esperamos a recibir un paquete en el buzón
        if (xQueueReceive(ui_queue, &rx_msg, portMAX_DELAY) == pdTRUE) {
            if (bsp_display_lock(pdMS_TO_TICKS(1000))) {
                
                // Si es de fuera, pintamos en la etiqueta exterior
                if (rx_msg.type == MSG_EXTERIOR) {
                    lv_label_set_text_fmt(s_ext_label, "Ext: %s C", rx_msg.text);
                } 
                // Si es de dentro, pintamos en la etiqueta interior
                else if (rx_msg.type == MSG_INTERIOR) {
                    lv_label_set_text_fmt(s_int_label, "Int: %s C", rx_msg.text);
                }
                
                bsp_display_unlock();
            }
        }
    }
}

// ---------------------------------------------------------
// INICIALIZACIÓN DE LA UI
// ---------------------------------------------------------
esp_err_t lvgl_beebotte_ui_init(void) {
    // Creamos el buzón para paquetes tipo "ui_msg_t"
    ui_queue = xQueueCreate(10, sizeof(ui_msg_t));
    if (ui_queue == NULL) {
        ESP_LOGE(TAG, "Error creando la cola de UI");
        return ESP_FAIL;
    }

    bsp_display_lock(0);
    lv_obj_t *scr = lv_scr_act();
    
    // Título un poco más arriba
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Termostato IoT");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -35); 

    // Etiqueta INTERIOR (Centro-arriba)
    s_int_label = lv_label_create(scr);
    lv_label_set_text(s_int_label, "Int: Leyendo...");
    lv_obj_set_style_text_font(s_int_label, &lv_font_montserrat_14, 0); 
    lv_obj_align(s_int_label, LV_ALIGN_CENTER, 0, -5); 

    // Etiqueta EXTERIOR (Centro-abajo)
    s_ext_label = lv_label_create(scr);
    lv_label_set_text(s_ext_label, "Ext: Esperando...");
    lv_obj_set_style_text_font(s_ext_label, &lv_font_montserrat_14, 0); 
    lv_obj_align(s_ext_label, LV_ALIGN_CENTER, 0, 20); 
    
    bsp_display_unlock();

    s_ui_ready = true;

    xTaskCreate(ui_updater_task, "ui_updater", 3072, NULL, 2, NULL);
    return ESP_OK;
}

// ---------------------------------------------------------
// FUNCIONES PARA ENVIAR AL BUZÓN
// ---------------------------------------------------------
void lvgl_beebotte_ui_update_value(const char *value_text) {
    if (!s_ui_ready || ui_queue == NULL || value_text == NULL) return;
    ui_msg_t msg;
    msg.type = MSG_EXTERIOR;
    strncpy(msg.text, value_text, sizeof(msg.text) - 1);
    msg.text[sizeof(msg.text) - 1] = '\0';
    xQueueSend(ui_queue, &msg, 0);
}

void lvgl_beebotte_ui_update_interior(float temp) {
    if (!s_ui_ready || ui_queue == NULL) return;
    ui_msg_t msg;
    msg.type = MSG_INTERIOR;
    snprintf(msg.text, sizeof(msg.text), "%.2f", temp);
    xQueueSend(ui_queue, &msg, 0);
}