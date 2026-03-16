idf_component_register(SRCS "main.c" "sensor_temperatura.c" "mqtt_beebotte.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt protocol_examples_common nvs_flash esp_netif esp_event driver)