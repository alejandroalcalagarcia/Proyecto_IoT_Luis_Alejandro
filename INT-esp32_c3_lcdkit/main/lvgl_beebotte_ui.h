#ifndef LVGL_BEEBOTTE_UI_H
#define LVGL_BEEBOTTE_UI_H

#include "esp_err.h"

esp_err_t lvgl_beebotte_ui_init(void);

// Actualiza la temperatura Exterior (viene de MQTT)
void lvgl_beebotte_ui_update_value(const char *value_text);

// Actualiza la temperatura Interior (viene del sensor AHT10)
void lvgl_beebotte_ui_update_interior(float temp);

#endif