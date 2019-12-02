#pragma once

#include <common.h>
#include "peripherals/base.h"

#define GPIO_BASE (PBASE + 0x00200000)

typedef struct {
    reg32 reserved;
    reg32 values[2];
} GpioOptionsRegs;

typedef struct {
    reg32 pinModes[6];
    GpioOptionsRegs outputSet;
    GpioOptionsRegs outputClear;
    GpioOptionsRegs pinLevel;
    GpioOptionsRegs eventDetectStatus;
    GpioOptionsRegs risingDetectEnable;
    GpioOptionsRegs fallingDetectEnable;
    GpioOptionsRegs highDetectEnable;
    GpioOptionsRegs lowDetectEnable;
    GpioOptionsRegs asyncRisingDetectEnable;
    GpioOptionsRegs asyncFallingDetectEnable;
    reg32 reserved0;
    reg32 pupdEnable;
    reg32 pupdClock0;
    reg32 pupdClock1;
    reg32 reserved1;
    reg32 test;
} GpioRegs;

typedef struct {
    reg32 irq;
    reg32 enables;
} AuxRegisters;

static volatile AuxRegisters *REGS_AUX;

inline AuxRegisters *AUX() {
    return REGS_AUX;
}

inline GpioRegs *GPIO() {
    return (GpioRegs *)GPIO_BASE;
}

typedef enum {
    PMIn = 0,
    PMOut = 1,
    PMAlt0 = 4,
    PMAlt1 = 5,
    PMAlt2 = 6,
    PMAlt3 = 7,
    PMAlt4 = 3,
    PMAlt5 = 2,
} PinMode;

void gpio_pin_mode(byte pinNumber, PinMode pm);
void gpio_enable_pupd();

void gpio_write(byte pinNumber, bool val);
