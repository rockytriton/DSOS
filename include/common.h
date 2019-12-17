#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef volatile dword reg32;

#define PACKED __attribute__ ((packed))

#define NULL 0

void assert(bool b, const char *msg);
