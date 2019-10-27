#include "timer.h"
#include "../drivers/mailbox/mailbox.h"
#include "peripherals/base.h"
#include "logger.h"
#include "utils.h"

#define CLOCKHZ	1000000

namespace dsos {

    struct TimerRegs {
        reg32 controlStatus;
        reg32 counterLow;
        reg32 counterHigh;
        reg32 compare[4];  
    };

    uint64_t systemTicks = 0;
    dword curValue = 0;
    dword interval = 0;
    dword timersReceived = 0;

    #define TIMER_CLO       (PBASE+0x00003004)
    TimerRegs *SYSTIMER = (TimerRegs *)(PBASE + 0x00003000);

    struct ClockData : public mailbox::Tag {
        dword clockId;
        dword rate;
    };

    dword getClockRate(ClockType ct) {

        ClockData cd;
        cd.clockId = ct;
        cd.rate = 0;

        mailbox::Mailbox::inst()->processTag(PROPTAG_GET_CLOCK_RATE, cd, sizeof(cd));

        return cd.rate;

    }

    Logger &logger = Logger::inst();

    extern "C" void handle_timer_irq( void ) 
    {
        curValue += (interval);

        SYSTIMER->compare[3] = curValue;
        SYSTIMER->controlStatus &= (1 << 3);
        systemTicks++;
    }

    void delayMs(dword ms) {
        qword start = systemTicks;
        qword end = start + ms;

        while(systemTicks < end);
    }

    void timerInit() {
        interval = CLOCKHZ / 1000;
        curValue = SYSTIMER->compare[3] = (SYSTIMER->counterLow + interval);
    }

}