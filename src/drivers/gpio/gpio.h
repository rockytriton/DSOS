#pragma once
#include "common.h"
#include "peripherals/base.h"

namespace dsos {
namespace gpio {

    const dword GPIO_BASE = PBASE + 0x00200000;

    struct GpioOptionsRegs {
        reg32 reserved;
        reg32 values[2];
    };

    struct GpioRegs {
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
    };

    inline GpioRegs *GPIO() {
        return (GpioRegs *)GPIO_BASE;
    }

    enum PinMode {
        PMIn = 0,
        PMOut = 1,
        PMAlt0 = 4,
        PMAlt1 = 5,
        PMAlt2 = 6,
        PMAlt3 = 7,
        PMAlt4 = 3,
        PMAlt5 = 2,
    };

    void setPinMode(byte pinNumber, PinMode pf);

}
}