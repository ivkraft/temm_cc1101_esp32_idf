
#include "esp_err.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"

esp_err_t shared_spi_init(spi_host_device_t host,
                          int sclk, int mosi, int miso,
                          int max_transfer_sz);

spi_host_device_t shared_spi_host(void);

// Глобальный mutex для всех операций на шине
void shared_spi_lock(void);
void shared_spi_unlock(void);

// Добавить SPI device (HW CS управляется драйвером)

esp_err_t shared_spi_init(spi_host_device_t host,
                          int pin_sclk,
                          int pin_mosi,
                          int pin_miso,
                          int max_transfer_sz);

