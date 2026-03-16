#ifndef SENSOR_TEMPERATURE_H
#define SENSOR_TEMPERATURE_H

#include "esp_err.h"

// Inicializa el bus I2C y el sensor AHT10
esp_err_t sensor_temperature_init(void);

// Lee el valor actual de temperatura y humedad
esp_err_t sensor_temperature_read(float *temperature, float *humidity);

#endif
