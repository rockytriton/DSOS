#pragma once
#include "peripherals/base.h"
#include "common.h"
#include "logger.h"

namespace dsos {
namespace drivers {

    struct AuxRegs {
        reg32 irq;
        reg32 enables;
    };

    struct MiniUartRegs {
        reg32 io;
        reg32 ier;
        reg32 iir;
        reg32 lcr;
        reg32 mcr;
        reg32 lsr;
        reg32 msr;
        reg32 scratch;
        reg32 control;
        reg32 status;
        reg32 baud;
    };

    class MiniUart {
    public:
        static MiniUart *inst() {
            if (!initialized) {
                instance->init();
                initialized = true;
            }
        }

        void send(char c);
        char recv();
        void send(const char *p);

    private:
        void init();

        static MiniUart *instance;
        static bool initialized;
    };

}
}
