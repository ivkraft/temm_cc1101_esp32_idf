#include "shared_spi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "driver/gpio.h"

#include "shared_spi.h"

#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "shared_spi";

static bool s_inited = false;
static spi_host_device_t s_host = (spi_host_device_t)-1;

esp_err_t shared_spi_init(spi_host_device_t host,
                          int pin_sclk,
                          int pin_mosi,
                          int pin_miso,
                          int max_transfer_sz)
{
    if (s_inited) {
        if (host != s_host) {
            ESP_LOGE(TAG, "SPI already inited on host=%d, requested host=%d", (int)s_host, (int)host);
            return ESP_ERR_INVALID_STATE;
        }
        return ESP_OK;
    }

    spi_bus_config_t buscfg = {
        .sclk_io_num = pin_sclk,
        .mosi_io_num = pin_mosi,
        .miso_io_num = pin_miso,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = max_transfer_sz,
    };

    esp_err_t err = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(err));
        return err;
    }

    s_inited = true;
    s_host = host;

    ESP_LOGI(TAG, "SPI bus inited host=%d sclk=%d mosi=%d miso=%d max_sz=%d",
             (int)host, pin_sclk, pin_mosi, pin_miso, max_transfer_sz);
    return ESP_OK;
}
