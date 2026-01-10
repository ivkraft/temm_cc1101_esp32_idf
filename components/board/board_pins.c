#include "board_pins.h"

static const board_pins_t s_pins = {
    // TFT
    .tft_sclk   = BOARD_LCD_PIN_SCLK,
    .tft_mosi   = BOARD_LCD_PIN_MOSI,
    .tft_miso   = BOARD_LCD_PIN_MISO,
    .tft_cs     = BOARD_LCD_PIN_CS,
    .tft_dc     = BOARD_LCD_PIN_DC,
    .tft_rst    = BOARD_LCD_PIN_RST,
    .tft_bl     = BOARD_LCD_PIN_BKLT,

    // CC1101
    .cc1101_sck  = BOARD_CC1101_PIN_SCK,
    .cc1101_mosi = BOARD_CC1101_PIN_MOSI,
    .cc1101_miso = BOARD_CC1101_PIN_MISO,
    .cc1101_cs   = BOARD_CC1101_PIN_CS,
    .cc1101_gdo0 = BOARD_CC1101_PIN_GDO0,
    .cc1101_gdo2 = BOARD_CC1101_PIN_GDO2,

    // Antenna switch
    .cc1101_sw1 = BOARD_CC1101_PIN_SW1,
    .cc1101_sw0 = BOARD_CC1101_PIN_SW0,

    // Power
    .power_on = BOARD_PIN_POWER_ON,

    
    .enc_a    = GPIO_NUM_4,
    .enc_b    = GPIO_NUM_5,
    .enc_key  = GPIO_NUM_0,
    .btn_back = GPIO_NUM_6,
};

const board_pins_t *board_get_pins(void) {
    return &s_pins;
}
