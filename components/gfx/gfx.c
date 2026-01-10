#include "gfx.h"
#include <stdlib.h>
#include <string.h>
#include "esp_heap_caps.h"

#include "display.h"
#include "board_pins.h"
#include "gfx_font8x8.h"

static uint16_t *s_fb = NULL;

static inline uint16_t W(void) { return (uint16_t)BOARD_LCD_HRES; }
static inline uint16_t H(void) { return (uint16_t)BOARD_LCD_VRES; }

void gfx_init(void) {
    if (s_fb) return;
    size_t n = (size_t)W() * (size_t)H();
s_fb = (uint16_t *)heap_caps_malloc(n * sizeof(uint16_t),
                                   MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (s_fb) memset(s_fb, 0, n * sizeof(uint16_t));
}

uint16_t gfx_w(void) { return W(); }
uint16_t gfx_h(void) { return H(); }

void gfx_clear(uint16_t c) {
    if (!s_fb) return;
    for (size_t i = 0, n = (size_t)W() * (size_t)H(); i < n; i++) s_fb[i] = c;
}

void gfx_pixel(int x, int y, uint16_t c) {
    if (!s_fb) return;
    if ((unsigned)x >= W() || (unsigned)y >= H()) return;
    s_fb[(size_t)y * W() + (size_t)x] = c;
}

void gfx_hline(int x, int y, int w, uint16_t c) {
    if (!s_fb) return;
    if ((unsigned)y >= H() || w <= 0) return;
    int x0 = x, x1 = x + w - 1;
    if (x1 < 0 || x0 >= (int)W()) return;
    if (x0 < 0) x0 = 0;
    if (x1 >= (int)W()) x1 = (int)W() - 1;
    uint16_t *p = &s_fb[(size_t)y * W() + (size_t)x0];
    for (int i = x0; i <= x1; i++) *p++ = c;
}

void gfx_vline(int x, int y, int h, uint16_t c) {
    if (!s_fb) return;
    if ((unsigned)x >= W() || h <= 0) return;
    int y0 = y, y1 = y + h - 1;
    if (y1 < 0 || y0 >= (int)H()) return;
    if (y0 < 0) y0 = 0;
    if (y1 >= (int)H()) y1 = (int)H() - 1;
    for (int yy = y0; yy <= y1; yy++) s_fb[(size_t)yy * W() + (size_t)x] = c;
}

// Bresenham
void gfx_line(int x0, int y0, int x1, int y1, uint16_t c) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0); // negative
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        gfx_pixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void gfx_rect(int x, int y, int w, int h, uint16_t c) {
    if (w <= 0 || h <= 0) return;
    gfx_hline(x, y, w, c);
    gfx_hline(x, y + h - 1, w, c);
    gfx_vline(x, y, h, c);
    gfx_vline(x + w - 1, y, h, c);
}

void gfx_fill_rect(int x, int y, int w, int h, uint16_t c) {
    if (!s_fb || w <= 0 || h <= 0) return;
    int x0 = x, x1 = x + w - 1;
    int y0 = y, y1 = y + h - 1;

    if (x1 < 0 || y1 < 0 || x0 >= (int)W() || y0 >= (int)H()) return;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= (int)W()) x1 = (int)W() - 1;
    if (y1 >= (int)H()) y1 = (int)H() - 1;

    int ww = x1 - x0 + 1;
    for (int yy = y0; yy <= y1; yy++) {
        uint16_t *p = &s_fb[(size_t)yy * W() + (size_t)x0];
        for (int i = 0; i < ww; i++) p[i] = c;
    }
}

void gfx_box(int x, int y, int w, int h, uint16_t fill, uint16_t border) {
    gfx_fill_rect(x, y, w, h, fill);
    gfx_rect(x, y, w, h, border);
}

esp_err_t gfx_present(void) {
    if (!s_fb) return ESP_ERR_INVALID_STATE;
    // flush whole screen (просто и надежно для меню)
    return display_flush(0, 0, (int)W() - 1, (int)H() - 1, s_fb);
}

#include "gfx.h"

// Подставь свою функцию пикселя.
// Если у тебя есть gfx_pixel(x,y,c) — просто замени вызовы putpixel() на gfx_pixel().
static inline void putpixel(int x, int y, uint16_t c) {
    // TODO: реализуй через твой framebuffer
    // пример (если у тебя есть fb и stride):
    // if ((unsigned)x < (unsigned)gfx_w() && (unsigned)y < (unsigned)gfx_h()) fb[y*stride + x] = c;
    extern void gfx_pixel(int x, int y, uint16_t c); // если уже есть
    gfx_pixel(x, y, c);
}

static const uint8_t font5x7[96][5] = {
    // ASCII 32..127, 5 колонок, биты по вертикали (LSB = верхний пиксель)
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00}, // '!'
    {0x00,0x07,0x00,0x07,0x00}, // '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // '$'
    {0x23,0x13,0x08,0x64,0x62}, // '%'
    {0x36,0x49,0x55,0x22,0x50}, // '&'
    {0x00,0x05,0x03,0x00,0x00}, // '''
    {0x00,0x1C,0x22,0x41,0x00}, // '('
    {0x00,0x41,0x22,0x1C,0x00}, // ')'
    {0x14,0x08,0x3E,0x08,0x14}, // '*'
    {0x08,0x08,0x3E,0x08,0x08}, // '+'
    {0x00,0x50,0x30,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08}, // '-'
    {0x00,0x60,0x60,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02}, // '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // '0'
    {0x00,0x42,0x7F,0x40,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46}, // '2'
    {0x21,0x41,0x45,0x4B,0x31}, // '3'
    {0x18,0x14,0x12,0x7F,0x10}, // '4'
    {0x27,0x45,0x45,0x45,0x39}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // '6'
    {0x01,0x71,0x09,0x05,0x03}, // '7'
    {0x36,0x49,0x49,0x49,0x36}, // '8'
    {0x06,0x49,0x49,0x29,0x1E}, // '9'
    {0x00,0x36,0x36,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14}, // '='
    {0x00,0x41,0x22,0x14,0x08}, // '>'
    {0x02,0x01,0x51,0x09,0x06}, // '?'
    {0x32,0x49,0x79,0x41,0x3E}, // '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 'Z'
    {0x00,0x7F,0x41,0x41,0x00}, // '['
    {0x02,0x04,0x08,0x10,0x20}, // '\'
    {0x00,0x41,0x41,0x7F,0x00}, // ']'
    {0x04,0x02,0x01,0x02,0x04}, // '^'
    {0x40,0x40,0x40,0x40,0x40}, // '_'
    {0x00,0x01,0x02,0x04,0x00}, // '`'
    {0x20,0x54,0x54,0x54,0x78}, // 'a'
    {0x7F,0x48,0x44,0x44,0x38}, // 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 'c'
    {0x38,0x44,0x44,0x48,0x7F}, // 'd'
    {0x38,0x54,0x54,0x54,0x18}, // 'e'
    {0x08,0x7E,0x09,0x01,0x02}, // 'f'
    {0x0C,0x52,0x52,0x52,0x3E}, // 'g'
    {0x7F,0x08,0x04,0x04,0x78}, // 'h'
    {0x00,0x44,0x7D,0x40,0x00}, // 'i'
    {0x20,0x40,0x44,0x3D,0x00}, // 'j'
    {0x7F,0x10,0x28,0x44,0x00}, // 'k'
    {0x00,0x41,0x7F,0x40,0x00}, // 'l'
    {0x7C,0x04,0x18,0x04,0x78}, // 'm'
    {0x7C,0x08,0x04,0x04,0x78}, // 'n'
    {0x38,0x44,0x44,0x44,0x38}, // 'o'
    {0x7C,0x14,0x14,0x14,0x08}, // 'p'
    {0x08,0x14,0x14,0x18,0x7C}, // 'q'
    {0x7C,0x08,0x04,0x04,0x08}, // 'r'
    {0x48,0x54,0x54,0x54,0x20}, // 's'
    {0x04,0x3F,0x44,0x40,0x20}, // 't'
    {0x3C,0x40,0x40,0x20,0x7C}, // 'u'
    {0x1C,0x20,0x40,0x20,0x1C}, // 'v'
    {0x3C,0x40,0x30,0x40,0x3C}, // 'w'
    {0x44,0x28,0x10,0x28,0x44}, // 'x'
    {0x0C,0x50,0x50,0x50,0x3C}, // 'y'
    {0x44,0x64,0x54,0x4C,0x44}, // 'z'
    {0x00,0x08,0x36,0x41,0x00}, // '{'
    {0x00,0x00,0x7F,0x00,0x00}, // '|'
    {0x00,0x41,0x36,0x08,0x00}, // '}'
    {0x08,0x04,0x08,0x10,0x08}, // '~'
    {0x00,0x00,0x00,0x00,0x00}, // DEL
};

int gfx_font_w(void) { return 6; }
int gfx_font_h(void) { return 8; }

int gfx_text_width(const char *s) {
    if (!s) return 0;
    int n = 0;
    while (*s++) n++;
    return n * gfx_font_w();
}


static inline void draw_px_scaled(int x, int y, uint16_t c, int s) {
    // нужен gfx_fill_rect()
    gfx_fill_rect(x, y, s, s, c);
}

void gfx_text_scale(int x, int y, const char *s, uint16_t c, int scale) {
    if (!s) return;
    if (scale < 1) scale = 1;

    int cx = x;
    while (*s) {
        uint8_t ch = (uint8_t)*s++;
        const uint8_t *glyph = gfx_font8x8_basic[ch];

        for (int row = 0; row < 8; row++) {
            uint8_t bits = glyph[row];
            for (int col = 0; col < 8; col++) {
                if (bits & (1u << (7 - col))) {
                    // bit0 = левый столбец (так удобнее для этого набора)
                    draw_px_scaled(cx + col * scale, y + row * scale, c, scale);
                }
            }
        }
        cx += 8 * scale + scale; // межбуквенный зазор = scale
    }
}

int gfx_text_width_scale(const char *s, int scale) {
    if (!s) return 0;
    if (scale < 1) scale = 1;
    int n = 0;
    while (*s++) n++;
    if (n == 0) return 0;
    return n * (8 * scale + scale) - scale;
}

static void draw_char_core(int x, int y, char ch, uint16_t fg, bool use_bg, uint16_t bg) {
    if ((unsigned char)ch < 32 || (unsigned char)ch > 127) ch = '?';
    const uint8_t *cols = font5x7[(unsigned char)ch - 32];

    // 5x7 + 1px spacing => 6x8
    for (int cx = 0; cx < 6; cx++) {
        uint8_t bits = (cx < 5) ? cols[cx] : 0x00; // 6-я колонка пустая (пробел)
        for (int cy = 0; cy < 8; cy++) {
            bool on = (cy < 7) ? ((bits >> cy) & 1) : false; // 8-я строка пустая
            if (on) {
                putpixel(x + cx, y + cy, fg);
            } else if (use_bg) {
                putpixel(x + cx, y + cy, bg);
            }
        }
    }
}

void gfx_char(int x, int y, char ch, uint16_t fg) {
    draw_char_core(x, y, ch, fg, false, 0);
}

void gfx_char_bg(int x, int y, char ch, uint16_t fg, uint16_t bg) {
    draw_char_core(x, y, ch, fg, true, bg);
}

void gfx_text(int x, int y, const char *s, uint16_t fg) {
    if (!s) return;
    int cx = x;
    while (*s) {
        if (*s == '\n') { y += gfx_font_h(); cx = x; s++; continue; }
        draw_char_core(cx, y, *s++, fg, false, 0);
        cx += gfx_font_w();
    }
}

void gfx_text_bg(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
    if (!s) return;
    int cx = x;
    while (*s) {
        if (*s == '\n') { y += gfx_font_h(); cx = x; s++; continue; }
        draw_char_core(cx, y, *s++, fg, true, bg);
        cx += gfx_font_w();
    }
}

