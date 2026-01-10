#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum {
    INPUT_EVT_ENC,      // v = delta (+/-)
    INPUT_EVT_KEY,      // v = 1 pressed
    INPUT_EVT_BACK,     // v = 1 pressed
} input_evt_type_t;

typedef struct {
    input_evt_type_t type;
    int32_t v;
} input_evt_t;

void input_init(void);
QueueHandle_t input_get_queue(void);
