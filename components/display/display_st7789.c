#include "display.h"
#include "board_pins.h"

#include "esp_check.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_st7789.h"

static const char *TAG = "display";

static esp_lcd_panel_io_handle_t s_io    = NULL;
static esp_lcd_panel_handle_t    s_panel = NULL;

static void apply_rotation(esp_lcd_panel_handle_t panel, int rotation_deg)
{
    bool swap_xy = false;
    bool mirror_x = false;
    bool mirror_y = false;

    switch (rotation_deg) {
        case 0:   swap_xy = false; mirror_x = false; mirror_y = false; break;
        case 90:  swap_xy = true;  mirror_x = true;  mirror_y = false; break;
        case 180: swap_xy = false; mirror_x = true;  mirror_y = true;  break;
        case 270: swap_xy = true;  mirror_x = false; mirror_y = true;  break;
        default:
            ESP_LOGW(TAG, "Unsupported rotation=%d, using 0", rotation_deg);
            break;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel, swap_xy));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, mirror_x, mirror_y));
}

void display_backlight(bool on)
{
    const board_pins_t *p = board_get_pins();
    if (!p) return;

    if (p->tft_bl != GPIO_NUM_NC) {
        gpio_set_level(p->tft_bl, on ? BOARD_LCD_BKLT_ON_LEVEL : !BOARD_LCD_BKLT_ON_LEVEL);
    }
}

esp_err_t display_init(spi_host_device_t host)
{
    const board_pins_t *p = board_get_pins();
    ESP_RETURN_ON_FALSE(p != NULL, ESP_ERR_INVALID_STATE, TAG, "board_get_pins() returned NULL");

    // Backlight GPIO
    if (p->tft_bl != GPIO_NUM_NC) {
        gpio_config_t blcfg = {
            .pin_bit_mask = 1ULL << (uint32_t)p->tft_bl,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_RETURN_ON_ERROR(gpio_config(&blcfg), TAG, "gpio_config(backlight) failed");
        display_backlight(false);
    }

    // Panel IO over SPI (bus уже должен быть initialized снаружи)
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = (int)p->tft_dc,
        .cs_gpio_num = (int)p->tft_cs,          // CS управляется драйвером
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };

    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)host, &io_config, &s_io),
        TAG, "esp_lcd_new_panel_io_spi failed"
    );

    // ST7789 panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = (int)p->tft_rst,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
        // КЛЮЧЕВО: если у тебя “RGB565 bytes swapped” — это лечит мусор/цвета
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        // при необходимости можно менять порядок элементов
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(s_io, &panel_config, &s_panel),
                        TAG, "esp_lcd_new_panel_st7789 failed");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel), TAG, "panel_reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel),  TAG, "panel_init failed");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_set_gap(s_panel, BOARD_LCD_X_GAP, BOARD_LCD_Y_GAP),
                        TAG, "panel_set_gap failed");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(s_panel, BOARD_LCD_INVERT_COLOR ? true : false),
                        TAG, "panel_invert_color failed");

    apply_rotation(s_panel, BOARD_LCD_ROTATION);

    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel, true), TAG, "disp_on_off failed");

    display_backlight(true);

    ESP_LOGI(TAG, "Display init OK (ST7789 %dx%d rot=%d gap=%d,%d inv=%d)",
             BOARD_LCD_HRES, BOARD_LCD_VRES,
             BOARD_LCD_ROTATION, BOARD_LCD_X_GAP, BOARD_LCD_Y_GAP, BOARD_LCD_INVERT_COLOR);

    return ESP_OK;
}

esp_err_t display_flush(int x1, int y1, int x2, int y2, const void *color_data)
{
    ESP_RETURN_ON_FALSE(s_panel != NULL, ESP_ERR_INVALID_STATE, TAG, "panel not initialized");
    return esp_lcd_panel_draw_bitmap(s_panel, x1, y1, x2 + 1, y2 + 1, color_data);
}
