#include "common.h"

extern "C" unsigned long allocPage();
extern "C" void freePage(unsigned long p);

void memcpy(byte *dest, byte *source, dword len) {
    for (int i=0; i<len; i++) {
        dest[i] = source[i];
    }
}