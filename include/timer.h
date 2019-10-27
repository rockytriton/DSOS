#ifndef	_TIMER_H
#define	_TIMER_H

#include "common.h"

namespace dsos {

enum ClockType {
    CTEmmc = 1,
    CTUart = 2,
    CTArm = 3,
    CTCore = 4
};

dword getClockRate(ClockType ct);
void timerInit();
void delayMs(dword ms);

}

#endif  /*_TIMER_H */
