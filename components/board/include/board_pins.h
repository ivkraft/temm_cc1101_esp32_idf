#pragma once

#include "hal/gpio_types.h"
#include "hal/spi_types.h"   // spi_host_device_t

// ----------------------------------------------------------------------------
// Board ID
// ----------------------------------------------------------------------------
#define T_EMBED_1101
#if !defined(T_EMBED_1101)
#warning "T_EMBED_1101 is not defined. This header is intended for LilyGO T-Embed + CC1101."
#endif

// ----------------------------------------------------------------------------
// Pin map: LilyGO T-Embed + CC1101 (per your definitions)
// Shared SPI bus lines (TFT + CC1101)
// ----------------------------------------------------------------------------
#define BOARD_PIN_SPI_MOSI          GPIO_NUM_9
#define BOARD_PIN_SPI_MISO          GPIO_NUM_10
#define BOARD_PIN_SPI_SCLK          GPIO_NUM_11

// TFT ST7789
#define BOARD_LCD_PIN_CS            GPIO_NUM_41
#define BOARD_LCD_PIN_DC            GPIO_NUM_16
#define BOARD_LCD_PIN_RST           GPIO_NUM_40
#define BOARD_LCD_PIN_BKLT          GPIO_NUM_21
#define BOARD_LCD_PIN_SCLK          BOARD_PIN_SPI_SCLK
#define BOARD_LCD_PIN_MOSI          BOARD_PIN_SPI_MOSI
#define BOARD_LCD_PIN_MISO          GPIO_NUM_10   // ST7789 обычно не читает MISO

// CC1101
#define BOARD_CC1101_PIN_CS         GPIO_NUM_12
#define BOARD_CC1101_PIN_GDO0       GPIO_NUM_3
#define BOARD_CC1101_PIN_GDO2       GPIO_NUM_38
#define BOARD_CC1101_PIN_SCK        BOARD_PIN_SPI_SCLK
#define BOARD_CC1101_PIN_MOSI       BOARD_PIN_SPI_MOSI
#define BOARD_CC1101_PIN_MISO       BOARD_PIN_SPI_MISO


// Antenna switch (если реально используешь в коде)
#define BOARD_CC1101_PIN_SW1        GPIO_NUM_47
#define BOARD_CC1101_PIN_SW0        GPIO_NUM_48

// Power enable (как у тебя)
#define BOARD_PIN_POWER_ON          GPIO_NUM_15

// I2C Grove (как у тебя)
#define BOARD_I2C_SDA               GPIO_NUM_8
#define BOARD_I2C_SCL               GPIO_NUM_18

// ----------------------------------------------------------------------------
// LCD config (ST7789 170x320)
// ----------------------------------------------------------------------------
#define BOARD_LCD_SPI_HOST          SPI2_HOST

#define BOARD_LCD_HRES              320
#define BOARD_LCD_VRES              170

// Для 170x320 на ST7789 (контроллер 240x320) часто нужен X gap = 35
#define BOARD_LCD_X_GAP             0
#define BOARD_LCD_Y_GAP             35

// В твоём Arduino было ROTATION 3 (TFT_eSPI: 0..3). Это соответствует 270 градусам.
#define BOARD_LCD_ROTATION          270

#define BOARD_LCD_INVERT_COLOR      1

// Backlight on/off level
#define BOARD_LCD_BKLT_ON_LEVEL     1

#define PIN_POWER_EN 15 // Проверьте схему вашей ревизии T-Embed


// ----------------------------------------------------------------------------
// board_get_pins() API (единый источник истины для компонентов)
// ----------------------------------------------------------------------------
typedef struct {
    // TFT
    gpio_num_t tft_sclk;
    gpio_num_t tft_mosi;
    gpio_num_t tft_miso;
    gpio_num_t tft_cs;
    gpio_num_t tft_dc;
    gpio_num_t tft_rst;
    gpio_num_t tft_bl;

    // CC1101
    gpio_num_t cc1101_sck;
    gpio_num_t cc1101_mosi;
    gpio_num_t cc1101_miso;
    gpio_num_t cc1101_cs;
    gpio_num_t cc1101_gdo0;
    gpio_num_t cc1101_gdo2;

    // Optional antenna switch
    gpio_num_t cc1101_sw1;
    gpio_num_t cc1101_sw0;

    // Power
    gpio_num_t power_on;

        // INPUTS
    gpio_num_t enc_a;
    gpio_num_t enc_b;
    gpio_num_t enc_key;   // encoder press
    gpio_num_t btn_back;  // back button
} board_pins_t;

const board_pins_t *board_get_pins(void);
