#pragma once

#include <common.h>
#include <sys/timer.h>

inline bool registerWait(reg32 *reg, dword mask, bool set, int timeout) {
    while(--timeout > 0) {
        if ((*reg & mask) ? set : !set) {
            return true;
        }

        timer_delay(1);
    }

    return timeout > 0;
}

inline bool flag32_set(dword n, byte index) {
    return ((n & (1 << index)) != 0);
}

inline bool flag64_set(qword n, byte index) {
    return ((n & (1 << index)) != 0);
}
