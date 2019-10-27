#pragma once

#include "common.h"

namespace dsos {
namespace emmc {

    struct EmmcRegs {
        reg32 arg2;
        reg32 blockSizeCount;
        reg32 arg1;
        reg32 commandTransferMode;
        reg32 response[4];
        reg32 data;
        reg32 status;
        reg32 control[2];
        reg32 irqFlags;
        reg32 irqMask;
        reg32 irqEnable;
        reg32 control2;
        reg32 cap1;
        reg32 cap2;
        reg32 res0[2];
        reg32 forceIrq;
        reg32 res1[7];
        reg32 bootTimeout;
        reg32 debugConfig;
        reg32 res2[2];
        reg32 extFifoConfig;
        reg32 extFifoEnable;
        reg32 tuningStep;
        reg32 tuningStepSDR;
        reg32 tuningStepDDR;
        reg32 res3[23];
        reg32 spiIrqSupport;
        reg32 res4[2];
        reg32 slotIrqStatus;
    };

    class Emmc {
    public:
        bool cardReset();
        bool cardInit();
        bool command(dword commandReg, dword arg, dword timeout);
        
    private:
        bool issueCommand(dword commandReg, dword arg, dword timeout);

        dword transferBlocks = 0;
        reg32 lastCommandReg = 0;
        dword lastSuccess = 0;
        dword blockSize = 0;
    };
}
}