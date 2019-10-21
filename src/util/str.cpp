
#include "util/str.h"

namespace dsos {

dword stringLen(const char *p) {
    int len = 0;
    for (int i=0; p[i] != 0; i++) {
        if (p[i] == 0) {
            break;
        }

        len++;
    }

    return len;
}

dword copyString(char *dest, const char *source) {
    int len = stringLen(source);

    for (int i=0; i<len; i++) {
        dest[i] = source[i];
    }

    dest[len] = 0;
}

void toString(dword num, byte base, bool upper, String &s) {
    int n = 0;
    int d = 1;
    char buff[64];
    char *bf = buff;

    while(num / d >= base) {
        d *= base;
    }

    while(d != 0) {
        int digit = num / d;
        num %= d;
        d /= base;

        if (n || digit > 0 || digit == 0) {
            *bf++ = digit + (digit < 10 ? '0' : (upper ? 'A' : 'a') - 10);
            n++;
        }

        *bf = 0;
    }

    s += (const char *)buff;
}

void toString(signed int num, String &s) {
    if (num < 0) {
        s = "-";
    }

    toString(num, 10, false, s);
}

int String::size(const char *p) {
    return stringLen(p);
}

}