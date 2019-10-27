#pragma once

#include "common.h"

namespace dsos {
namespace spi {

    struct SPIRegs {
        union {
            reg32 value;
            struct {
                byte chipSelect : 2;
                byte clockPhase : 1;
                byte clockPolarity : 1;
                byte clearFifoTx : 1;
                byte clearFifoRx : 1;
                byte csPolarity : 1;
                byte transferActive : 1;
                byte dmaEnable : 1;
                byte interruptOnDone : 1;
                byte interruptOnRxr : 1;
                byte autoDeassertCS : 1;
                byte readEnable : 1;
                byte lossiEnable : 1;
                byte unused0 : 2;
                byte done : 1;
                byte rxReady : 1;
                byte txReady : 1;
                byte rxNeedsRead : 1;
                byte rxFull : 1;
                byte csPolarity0 : 1;
                byte csPolarity1 : 1;
                byte csPolarity2 : 1;
                byte dmaLossiEnabled : 1;
                byte lossiLong : 1;
                byte reserved : 6;
            } fields;
        } controlStatus;
        reg32 data;
        reg32 clockDivider;
        reg32 dataLength;
        reg32 lossiMode;
        reg32 dmaControls;
    };

    class SPI {
    public:
        void setChipSelect(byte cs) {  
            regs->controlStatus &= ~3;
            regs->controlStatus |= cs;
        }

        void setClockPolarity(bool high) {
            setFlag(regs->controlStatus, 3, high);
        }

        void setPhase(bool beginning) {
            setFlag(regs->controlStatus, 2, beginning);
        }

        void fifoClear(bool tx, bool rx) {
            setFlag(regs->controlStatus, 5, rx);
            setFlag(regs->controlStatus, 4, tx);
        }

        void setCSPolarity(bool high) {
            setFlag(regs->controlStatus, 6, high);
        }

    private:
        SPIRegs *regs;
    };
}
}