#pragma once
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

void gfx_init(void); // выделяет framebuffer
uint16_t gfx_w(void);
uint16_t gfx_h(void);

void gfx_clear(uint16_t c);
void gfx_pixel(int x, int y, uint16_t c);

void gfx_hline(int x, int y, int w, uint16_t c);
void gfx_vline(int x, int y, int h, uint16_t c);
void gfx_line(int x0, int y0, int x1, int y1, uint16_t c);

void gfx_rect(int x, int y, int w, int h, uint16_t c);
void gfx_fill_rect(int x, int y, int w, int h, uint16_t c);
void gfx_box(int x, int y, int w, int h, uint16_t fill, uint16_t border);
int gfx_font_w(void); // 6
int gfx_font_h(void); // 8
int gfx_text_width(const char *s);

void gfx_char(int x, int y, char ch, uint16_t fg);                       // прозрачный фон
void gfx_char_bg(int x, int y, char ch, uint16_t fg, uint16_t bg);       // с фоном
void gfx_text(int x, int y, const char *s, uint16_t fg);                 // прозрачный фон
void gfx_text_bg(int x, int y, const char *s, uint16_t fg, uint16_t bg); // с фоном

void gfx_text_scale(int x, int y, const char *s, uint16_t c, int scale);

int gfx_text_width(const char *s); // (если уже есть — ок)
int gfx_text_width_scale(const char *s, int scale);

esp_err_t gfx_present(void); // flush whole screen

// Цвета: ВАЖНО — у тебя байты swapped (красный = 0x00F8).
static inline uint16_t gfx_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); // 0xF800 red before swap
    return (uint16_t)((c >> 8) | (c << 8));                        // byteswap16
}
