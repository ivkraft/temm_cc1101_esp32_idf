#include "board_pins.h"
#include "hal/gpio_types.h"  // gpio_num_t
#include "hal/spi_types.h"   // SPI2_HOST / SPI3_HOST



static const board_pins_t s_pins = {
    // заполни реальными GPIO под T-Embed S3 + CC1101
};

const board_pins_t *board_get_pins(void) {
    return &s_pins;
}
