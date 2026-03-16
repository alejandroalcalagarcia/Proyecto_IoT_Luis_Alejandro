#include "sensor_temperatura.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ATENCIÓN: Cambia estos pines si tienes conflicto con el monitor serie en el C3
#define I2C_MASTER_SDA_IO       20 
#define I2C_MASTER_SCL_IO       21 
#define I2C_MASTER_PORT         I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      100000
#define I2C_MASTER_TIMEOUT_MS   1000
#define AHT10_ADDR              0x38

static const char *TAG = "AHT10_INTEGRADO";

esp_err_t sensor_temperature_init(void) {
    ESP_LOGI(TAG, "Inicializando I2C...");
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_PORT, &conf));
    esp_err_t err = i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error instalando driver I2C");
        return err;
    }

    // Secuencia de inicialización del AHT10 del código original
    uint8_t init_cmd[3] = {0xE1, 0x08, 0x00};
    err = i2c_master_write_to_device(I2C_MASTER_PORT, AHT10_ADDR, init_cmd, sizeof(init_cmd), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "AHT10 inicializado correctamente");
    } else {
        ESP_LOGE(TAG, "Error inicializando AHT10");
    }

    // Pausa crucial de 500ms del código original
    vTaskDelay(pdMS_TO_TICKS(500));
    return err;
}

esp_err_t sensor_temperature_read(float *temperature, float *humidity) {
    if (!temperature || !humidity) return ESP_ERR_INVALID_ARG;

    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    uint8_t buf[6];

    esp_err_t err = i2c_master_write_to_device(I2C_MASTER_PORT, AHT10_ADDR, cmd, sizeof(cmd), pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error enviando comando de lectura");
        return err;
    }

    // Tiempo de espera para la medición (del código original)
    vTaskDelay(pdMS_TO_TICKS(80));
    
    err = i2c_master_read_from_device(I2C_MASTER_PORT, AHT10_ADDR, buf, 6, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));

    if (err == ESP_OK) {
        uint32_t raw_hum = ((uint32_t)(buf[1]) << 12) | ((uint32_t)(buf[2]) << 4) | ((buf[3] & 0xF0) >> 4);
        uint32_t raw_temp = (((uint32_t)(buf[3] & 0x0F)) << 16) | ((uint32_t)(buf[4]) << 8) | (uint32_t)(buf[5]);

        *humidity = ((float)raw_hum / 1048576.0f) * 100.0f;
        *temperature = (((float)raw_temp / 1048576.0f) * 200.0f) - 50.0f;
    } else {
        ESP_LOGE(TAG, "Error leyendo datos del sensor");
    }
    return err;
}