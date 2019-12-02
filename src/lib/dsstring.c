#include <lib/dsstring.h>

int str_len(char *p) {
    int count = 0;

    while(*p++ != 0) {
        count++;
    }

    return count;
}

char *str_copy(char *dest, char *src) {
    char *start = dest;

    do {
        *dest = *src;
        dest++;
    } while(*src++);

    return start;
}

char *str_cat(char *dest, char *src) {
    return NULL;
}

bool str_equal(char *a, char *b) {
    int lenA = str_len(a);
    int lenB = str_len(b);

    if (lenA != lenB) {
        return false;
    }

    for (int i=0; i<lenA; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

