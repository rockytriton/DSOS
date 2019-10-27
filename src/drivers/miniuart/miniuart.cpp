#include "miniuart.h"
#include "peripherals/gpio.h"
#include "utils.h"
#include "../gpio/gpio.h"

#define IER_REG_EN_REC_INT (1 << 0)
#define IER_REG_INT        (3 << 2) // Must be set to receive interrupts
#define IER_REG_VALUE      (IER_REG_EN_REC_INT | IER_REG_INT)

namespace dsos {
namespace drivers {

    MiniUart miniUart;

    static AuxRegs *REGS_AUX = (AuxRegs *)(PBASE + 0x00215000);
    static MiniUartRegs *REGS_MU = (MiniUartRegs *)(PBASE + 0x00215040);

    MiniUart *MiniUart::instance = &miniUart;
    bool MiniUart::initialized = false;

    void MiniUart::init() {
        gpio::setPinMode(14, gpio::PMAlt5);
        gpio::setPinMode(15, gpio::PMAlt5);

        gpio::setPinMode(18, gpio::PMOut);

        /*
        reg32 selector = get32(GPFSEL1);

        selector &= ~(7<<12);                   // clean gpio14
        selector |= 2<<12;                      // set alt5 for gpio14
        selector &= ~(7<<15);                   // clean gpio15
        selector |= 2<<15;                      // set alt5 for gpio15
        put32(GPFSEL1,selector);

        put32(GPPUD,0);
        delay(150);
        put32(GPPUDCLK0,(1<<14)|(1<<15));
        delay(150);
        put32(GPPUDCLK0,0);
        */

        gpio::GPIO()->pupdEnable = 0;
        delay(150);
        gpio::GPIO()->pupdClock0 |= (1 << 14) | (1 << 15);
        delay(150);
        gpio::GPIO()->pupdClock0 = 0;

        delay(150);
        gpio::GPIO()->pupdEnable = 2;
        delay(150);
        gpio::GPIO()->pupdClock0 |= (1 << 18);
        delay(150);
        gpio::GPIO()->pupdClock0 = 0;

        REGS_AUX->enables = 1;
        REGS_MU->control = 0;
        REGS_MU->ier = IER_REG_VALUE;
        REGS_MU->lcr = 3; //8 bit mode
        REGS_MU->mcr = 0;
        REGS_MU->baud = 270; //115200
        REGS_MU->control = 3; //reentable tx/rx
    }

    void MiniUart::send(const char *p) {
        while(*p != 0) {
            send(*p++);
        }
    }

    void MiniUart::send(char c) {
        while(!(REGS_MU->lsr & 0x20));

        REGS_MU->io = c;
    }

    char MiniUart::recv() {
        while(!(REGS_MU->lsr & 1));

        return REGS_MU->io & 0xFF;
    }


}
}

