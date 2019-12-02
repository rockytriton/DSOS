#pragma once
#include <stdint.h>

typedef uint8_t bool;
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef volatile dword reg32;

#define PACKED __attribute__ ((packed))

#define NULL 0
#define true 1
#define false 0

void assert(bool b, const char *msg);
