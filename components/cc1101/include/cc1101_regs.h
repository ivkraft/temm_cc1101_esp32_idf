#pragma once
#include <stdint.h>

// CC1101 register addresses (Config registers)
#define CC1101_IOCFG0    0x02
#define CC1101_FSCTRL1   0x0B

#define CC1101_PKTCTRL1  0x07
#define CC1101_PKTCTRL0  0x08

#define CC1101_MDMCFG4   0x10
#define CC1101_MDMCFG3   0x11
#define CC1101_MDMCFG2   0x12
#define CC1101_MDMCFG1   0x13
#define CC1101_MDMCFG0   0x14
#define CC1101_DEVIATN   0x15

#define CC1101_MCSM0     0x18
#define CC1101_FOCCFG    0x19

#define CC1101_AGCCTRL2  0x1B
#define CC1101_AGCCTRL1  0x1C
#define CC1101_AGCCTRL0  0x1D

#define CC1101_WORCTRL   0x20

#define CC1101_FREND1    0x21
#define CC1101_FREND0    0x22

// PATABLE
#define CC1101_PATABLE   0x3E
