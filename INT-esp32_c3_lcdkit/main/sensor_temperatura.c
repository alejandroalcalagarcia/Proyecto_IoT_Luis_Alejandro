#include "sensor_temperatura.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SDA_IO       20
#define I2C_MASTER_SCL_IO       21
#define I2C_MASTER_PORT         I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      100000
#define AHT10_ADDR              0x38

static const char *TAG = "AHT10_SIMPLE";

esp_err_t sensor_temperature_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_PORT, &conf);
    i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);

    uint8_t init_cmd[3] = {0xE1, 0x08, 0x00};
    return i2c_master_write_to_device(I2C_MASTER_PORT, AHT10_ADDR, init_cmd, 3, pdMS_TO_TICKS(1000));
}

esp_err_t sensor_temperature_read(float *temperature, float *humidity) {
    uint8_t cmd[3] = {0xAC, 0x33, 0x00};
    uint8_t buf[6];

    i2c_master_write_to_device(I2C_MASTER_PORT, AHT10_ADDR, cmd, 3, pdMS_TO_TICKS(1000));
    vTaskDelay(pdMS_TO_TICKS(80));
    esp_err_t err = i2c_master_read_from_device(I2C_MASTER_PORT, AHT10_ADDR, buf, 6, pdMS_TO_TICKS(1000));

    if (err == ESP_OK) {
        uint32_t raw_hum  = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | (buf[3] >> 4);
        uint32_t raw_temp = (((uint32_t)(buf[3] & 0x0F)) << 16) | ((uint32_t)buf[4] << 8) | buf[5];
        *humidity    = (float)raw_hum  / 1048576.0f * 100.0f;
        *temperature = (float)raw_temp / 1048576.0f * 200.0f - 50.0f;
    }
    return err;
}
