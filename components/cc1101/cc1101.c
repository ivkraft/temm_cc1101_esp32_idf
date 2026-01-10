#include "cc1101.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "cc1101";

#define CC1101_READ_BURST  0xC0

static esp_err_t xfer(cc1101_t *cc, const uint8_t *tx, uint8_t *rx, int len)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    return spi_device_polling_transmit(cc->dev, &t);
}

esp_err_t cc1101_init_dev(cc1101_t *cc, const cc1101_cfg_t *cfg)
{
    if (!cc || !cfg) return ESP_ERR_INVALID_ARG;
    memset(cc, 0, sizeof(*cc));
    cc->host = cfg->host;
    cc->pin_cs = cfg->pin_cs;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (cfg->clock_hz > 0) ? cfg->clock_hz : (2 * 1000 * 1000),
        .mode = 0,
        .spics_io_num = cfg->pin_cs, // CS управляет SPI driver
        .queue_size = 1,
        .flags = SPI_DEVICE_NO_DUMMY,
    };

    esp_err_t err = spi_bus_add_device(cfg->host, &devcfg, &cc->dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "CC1101 dev added host=%d cs=%d", (int)cfg->host, cfg->pin_cs);
    return ESP_OK;
}

esp_err_t cc1101_strobe(cc1101_t *cc, uint8_t strobe)
{
    uint8_t tx[1] = { strobe };
    return xfer(cc, tx, NULL, 1);
}

esp_err_t cc1101_read_status(cc1101_t *cc, uint8_t addr, uint8_t *outv)
{
    if (!outv) return ESP_ERR_INVALID_ARG;
    uint8_t tx[2] = { (uint8_t)(addr | CC1101_READ_BURST), 0x00 };
    uint8_t rx[2] = { 0, 0 };
    esp_err_t err = xfer(cc, tx, rx, 2);
    if (err == ESP_OK) *outv = rx[1];
    return err;
}
