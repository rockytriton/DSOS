#pragma once

#include <cstdint>

typedef std::uint8_t byte;
typedef std::uint16_t word;
typedef std::uint32_t dword;
typedef std::uint64_t qword;

typedef volatile dword reg32;
typedef volatile word reg16;
typedef volatile byte reg8;

namespace dsos {
    inline void setFlag(reg32 reg, byte flag, bool value) {
        if (value) {
            reg |= (1 << flag);
        } else {
            reg &= ~(1 << flag);
        }
    }
}