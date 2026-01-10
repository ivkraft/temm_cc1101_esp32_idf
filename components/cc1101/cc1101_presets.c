#include "cc1101_presets.h"
#include "cc1101_regs.h"

const uint8_t subghz_device_cc1101_preset_2fsk_dev47_6khz_async_regs[] = {
    /* GPIO GD0 */
    CC1101_IOCFG0,   0x0D,  // GD0 async serial

    /* Frequency Synthesizer Control */
    CC1101_FSCTRL1,  0x06,  // IF

    /* Packet engine */
    CC1101_PKTCTRL0, 0x32,  // Async, continuous, no whitening
    CC1101_PKTCTRL1, 0x04,

    /* Modem Configuration */
    CC1101_MDMCFG0,  0x00,
    CC1101_MDMCFG1,  0x02,
    CC1101_MDMCFG2,  0x04,  // 2-FSK, no preamble/sync
    CC1101_MDMCFG3,  0x83,  // ~4.8 kBaud
    CC1101_MDMCFG4,  0x67,  // Rx BW ~270kHz
    CC1101_DEVIATN,  0x47,  // Dev ~47.6kHz

    /* Main Radio Control State Machine */
    CC1101_MCSM0,    0x18,  // autocal on idle->rx/tx

    /* Frequency Offset Compensation Configuration */
    CC1101_FOCCFG,   0x16,

    /* AGC */
    CC1101_AGCCTRL0, 0x91,
    CC1101_AGCCTRL1, 0x00,
    CC1101_AGCCTRL2, 0x07,

    /* WOR */
    CC1101_WORCTRL,  0xFB,

    /* Frontend */
    CC1101_FREND0,   0x10,
    CC1101_FREND1,   0x56,

    /* End marker */
    0, 0,

    /* PATABLE (8 bytes) */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
