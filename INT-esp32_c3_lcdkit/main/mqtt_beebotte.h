#ifndef MQTT_BEEBOTTE_H
#define MQTT_BEEBOTTE_H

#include "mqtt_client.h"
#include "esp_err.h"

// Inicializa la conexión MQTT a Beebotte
esp_mqtt_client_handle_t mqtt_beebotte_init(void);

#endif
