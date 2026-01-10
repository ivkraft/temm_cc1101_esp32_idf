// input.c — энкодер + кнопки с антидребезгом и “detent” (шагами)
#include "input.h"
#include "board_pins.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_timer.h"

// ---- ТЮНИНГ ----
// Фильтр по времени для A/B (микросекунды). 800..2500 обычно хватает.

// Сколько валидных gray-переходов считать за 1 “щелчок”.
// Часто: 4 (полный цикл), иногда: 2.


#define ENC_DETENT_STEPS       2
#define ENC_EDGE_DEBOUNCE_US   300

// Дребезг кнопок (микросекунды)
#define KEY_DEBOUNCE_US        15000
#define BACK_DEBOUNCE_US       15000

static QueueHandle_t s_q;

static volatile uint8_t s_prev_ab;
static volatile int32_t s_enc_accum;
static volatile int64_t s_last_enc_us;

static volatile int64_t s_last_key_us;
static volatile int64_t s_last_back_us;

static inline void IRAM_ATTR push_evt_isr(input_evt_type_t t, int32_t v) {
    if (!s_q) return;
    input_evt_t e = {.type = t, .v = v};
    BaseType_t hp = pdFALSE;
    xQueueSendFromISR(s_q, &e, &hp);
    if (hp) portYIELD_FROM_ISR();
}

static inline bool IRAM_ATTR enc_time_ok(int64_t now_us) {
    int64_t dt = now_us - s_last_enc_us;
    if (dt < ENC_EDGE_DEBOUNCE_US) return false;
    s_last_enc_us = now_us;
    return true;
}

static inline void IRAM_ATTR enc_accum_push(int dir) {
    // Если направление поменялось — сбросить накопитель (чтобы не “прыгало”)
    if ((s_enc_accum > 0 && dir < 0) || (s_enc_accum < 0 && dir > 0)) {
        s_enc_accum = 0;
    }

    s_enc_accum += dir;

    if (s_enc_accum >= ENC_DETENT_STEPS) {
        s_enc_accum = 0;
        push_evt_isr(INPUT_EVT_ENC, +1);
    } else if (s_enc_accum <= -ENC_DETENT_STEPS) {
        s_enc_accum = 0;
        push_evt_isr(INPUT_EVT_ENC, -1);
    }
}

static void IRAM_ATTR isr_enc(void *arg) {
    const board_pins_t *p = (const board_pins_t *)arg;

    uint8_t a = (uint8_t)gpio_get_level(p->enc_a);
    uint8_t b = (uint8_t)gpio_get_level(p->enc_b);
    uint8_t ab = (uint8_t)((a << 1) | b);

    // Gray-code transition table: index = (prev<<2)|cur, result = -1/0/+1
    static const int8_t t[16] = {
         0, -1, +1,  0,
        +1,  0,  0, -1,
        -1,  0,  0, +1,
         0, +1, -1,  0
    };

    int8_t d = t[(s_prev_ab << 2) | ab];
    s_prev_ab = ab;
    if (!d) return;

    int64_t now = esp_timer_get_time();
    if (!enc_time_ok(now)) return;

    // Выдаём событие только по “щелчкам”
    enc_accum_push((int)d);
}

static void IRAM_ATTR isr_key(void *arg) {
    const board_pins_t *p = (const board_pins_t *)arg;

    int64_t now = esp_timer_get_time();
    if (now - s_last_key_us < KEY_DEBOUNCE_US) return;
    s_last_key_us = now;

    // BTN_ACT LOW: pressed = 0
    if (gpio_get_level(p->enc_key) == 0) push_evt_isr(INPUT_EVT_KEY, 1);
}

static void IRAM_ATTR isr_back(void *arg) {
    const board_pins_t *p = (const board_pins_t *)arg;

    int64_t now = esp_timer_get_time();
    if (now - s_last_back_us < BACK_DEBOUNCE_US) return;
    s_last_back_us = now;

    if (gpio_get_level(p->btn_back) == 0) push_evt_isr(INPUT_EVT_BACK, 1);
}

void input_init(void) {
    const board_pins_t *p = board_get_pins();

    s_q = xQueueCreate(32, sizeof(input_evt_t));

    // Encoder A/B inputs
    gpio_config_t enc = {
        .pin_bit_mask = (1ULL << p->enc_a) | (1ULL << p->enc_b),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,      // BTN_ACT LOW
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&enc);

    // Key + Back inputs
    gpio_config_t btn = {
        .pin_bit_mask = (1ULL << p->enc_key) | (1ULL << p->btn_back),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,        // pressed -> LOW
    };
    gpio_config(&btn);

    // init states
    uint8_t a = (uint8_t)gpio_get_level(p->enc_a);
    uint8_t b = (uint8_t)gpio_get_level(p->enc_b);
    s_prev_ab = (uint8_t)((a << 1) | b);

    s_enc_accum   = 0;
    s_last_enc_us = 0;
    s_last_key_us = 0;
    s_last_back_us = 0;

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    gpio_isr_handler_add(p->enc_a,   isr_enc,  (void *)p);
    gpio_isr_handler_add(p->enc_b,   isr_enc,  (void *)p);
    gpio_isr_handler_add(p->enc_key, isr_key,  (void *)p);
    gpio_isr_handler_add(p->btn_back,isr_back, (void *)p);
}

QueueHandle_t input_get_queue(void) {
    return s_q;
}
