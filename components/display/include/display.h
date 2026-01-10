#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"   // spi_host_device_t

esp_err_t display_init(spi_host_device_t host);
void      display_backlight(bool on);
esp_err_t display_flush(int x1, int y1, int x2, int y2, const void *color_data);