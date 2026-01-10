// main/app_main.c  (ESP-IDF)
//
// Shared SPI bus + 2 devices:
//  - TFT as HW-CS device (optional)
//  - CC1101 as device too, BUT CS is manual (spics_io_num = -1)
//
// IMPORTANT:
//  - Do NOT "probe" / gpio_config() the TFT CS pin if TFT uses HW-CS.
//  - CC1101 CS is manual here by design.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

#include "board_pins.h"
#include "cc1101.h"
#include "display.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "gfx.h"
#include "input.h"
#include "shared_spi.h"

// ---------------- Pins (your T-Embed CC1101 wiring) ----------------
#define PIN_SPI_MOSI 9
#define PIN_SPI_MISO 10
#define PIN_SPI_SCLK 11

#define PIN_TFT_CS 41 // optional, HW-CS device
#define PIN_CC_CS 12  // manual CS
#define PIN_CC_GDO0 3
#define PIN_CC_GDO2 38

#define CC1101_MDMCFG2 0x12 /** Modem configuration */
#define CC1101_MDMCFG1 0x13 /** Modem configuration */
#define CC1101_MDMCFG0 0x14 /** Modem configuration */

// SPI host (host=1 in your logs)
#define SPI_HOST SPI2_HOST

// Clocks
#define TFT_SPI_HZ (40 * 1000 * 1000) // doesn't matter if you don't talk to TFT here
#define CC_SPI_HZ (2 * 1000 * 1000)

// ---------------- CC1101 minimal defs ----------------
#define CC1101_SRES 0x30

// Status regs (read with addr | 0xC0)
#define CC1101_PARTNUM 0x30
#define CC1101_VERSION 0x31
#define CC1101_MARCSTATE 0x35
#define CC1101_RSSI 0x34

#define CC1101_READ_SINGLE 0x80
#define CC1101_READ_BURST 0xC0

#define CC1101_SRX 0x34 // Стороб-команда перехода в RX

static const char *TAG = "MAIN";

void cc1101_power_on(bool on) {

    gpio_config_t pwr_cfg = {
        .pin_bit_mask = 1ULL << PIN_POWER_EN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&pwr_cfg);
    gpio_set_level(PIN_POWER_EN, on); // ВКЛЮЧАЕМ ПИТАНИЕ cc1101
    vTaskDelay(pdMS_TO_TICKS(50));
}

// ---------------- app_main ----------------
void app_main(void) {
    ESP_LOGI(TAG, "BOOT. Open serial now...");
    vTaskDelay(pdMS_TO_TICKS(2500));
    cc1101_power_on(true);

    const board_pins_t *p = board_get_pins();
    ESP_LOGI(
        TAG,
        "PINS: SCLK=%d MOSI=%d MISO=%d | TFT_CS=%d CC_CS=%d",
        (int)p->tft_sclk,
        (int)p->tft_mosi,
        (int)p->tft_miso,
        (int)p->tft_cs,
        (int)p->cc1101_cs
    );

    // 1) общий SPI bus (один раз)
    ESP_ERROR_CHECK(shared_spi_init(
        BOARD_LCD_SPI_HOST,
        (int)p->tft_sclk,
        (int)p->tft_mosi,
        (int)p->tft_miso,
        (BOARD_LCD_HRES * 40 * 2) + 64
    ));

    // 2) display (без spi_bus_initialize внутри!)
    ESP_ERROR_CHECK(display_init(BOARD_LCD_SPI_HOST));

    // 3) cc1101 device (на том же bus)
    cc1101_t cc;
    cc1101_cfg_t cccfg = {
        .host = BOARD_LCD_SPI_HOST,
        .pin_cs = (int)p->cc1101_cs,
        .clock_hz = 2 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(cc1101_init_dev(&cc, &cccfg));

    // мягкий reset стробом (минимум)
    ESP_ERROR_CHECK(cc1101_strobe(&cc, CC1101_SRES));
    vTaskDelay(pdMS_TO_TICKS(5));

    uint8_t part = 0, ver = 0, marc = 0;
    ESP_ERROR_CHECK(cc1101_read_status(&cc, CC1101_PARTNUM, &part));
    ESP_ERROR_CHECK(cc1101_read_status(&cc, CC1101_VERSION, &ver));
    ESP_ERROR_CHECK(cc1101_read_status(&cc, CC1101_MARCSTATE, &marc));
    ESP_LOGI(TAG, "CC1101 PART=0x%02X VER=0x%02X MARC=0x%02X", part, ver, marc);

    // 4) рисуем тестовый экран
    // gfx_init();
    // gfx_clear(gfx_rgb565(10,12,16));
    // gfx_text(10, 10, "Hello ST7789", gfx_rgb565(235,235,235));
    // gfx_present();

    const uint16_t bg = gfx_rgb565(10, 12, 16);
    const uint16_t panel = gfx_rgb565(18, 22, 28);
    const uint16_t accent = gfx_rgb565(0, 120, 255);
    const uint16_t text = gfx_rgb565(235, 235, 235);
    const uint16_t dim = gfx_rgb565(160, 160, 160);
    gfx_init();
    const int W = gfx_w();
    const int H = gfx_h();
    char char_array[12];
    snprintf(char_array, sizeof(char_array), "MARC=0x%02X", marc);
    gfx_clear(bg);

    gfx_fill_rect(6, 6, W - 12, H - 12, panel);
    gfx_rect(6, 6, W - 12, H - 12, gfx_rgb565(60, 70, 85));

    gfx_text_scale(14, 14, "MENU", text, 3);
    gfx_text_scale(14, 14 + 8 * 3 + 4, char_array, text, 3);
    // gfx_line(12, 14 + 8 * 3 + 10, W - 13, 14 + 8 * 3 + 10, gfx_rgb565(60, 70, 85));

    const int scale = 2;
    const int item_h = 8 * scale + 10;
    int y0 = 14 + 8 * 3 + 18;

    const char *hint = "Rotate: select   Press: enter";
    int hw = gfx_text_width_scale(hint, 1);
    gfx_text_scale((W - hw) / 2, H - 18, hint, gfx_rgb565(120, 130, 145), 1);

    ESP_ERROR_CHECK(gfx_present());
    int i = 0;
    char char_array2[12];

    while (1) {
        gfx_clear(bg);
        gfx_fill_rect(6, 6, W - 12, H - 12, panel);
        gfx_rect(6, 6, W - 12, H - 12, gfx_rgb565(60, 70, 85));
        marc = 0;
        ESP_ERROR_CHECK(cc1101_read_status(&cc, CC1101_MARCSTATE, &marc));
        snprintf(char_array, sizeof(char_array), "MARC=0x%02X", marc);
        snprintf(char_array2, sizeof(char_array2), "%d", i);
        gfx_text_scale(14, 14, char_array2, text, 3);

        gfx_text_scale(14, 14 + 8 * 3 + 4, char_array, text, 3);
        ESP_ERROR_CHECK(gfx_present());

        vTaskDelay(pdMS_TO_TICKS(500));
        i++;
    }
}
