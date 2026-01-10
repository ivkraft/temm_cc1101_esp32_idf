#pragma once
#include "esp_err.h"
#include "driver/spi_master.h"

typedef struct {
    spi_host_device_t host;
    int pin_cs;
    int clock_hz; // например 2*1000*1000
} cc1101_cfg_t;

typedef struct {
    spi_host_device_t host;
    int pin_cs;
    spi_device_handle_t dev;
} cc1101_t;

// STROBES
#define CC1101_SRES     0x30

// STATUS REGS (read with burst bit 0xC0)
#define CC1101_PARTNUM  0x30
#define CC1101_VERSION  0x31
#define CC1101_MARCSTATE 0x35
#define CC1101_RSSI     0x34

esp_err_t cc1101_init_dev(cc1101_t *cc, const cc1101_cfg_t *cfg);
esp_err_t cc1101_strobe(cc1101_t *cc, uint8_t strobe);
esp_err_t cc1101_read_status(cc1101_t *cc, uint8_t addr, uint8_t *outv);