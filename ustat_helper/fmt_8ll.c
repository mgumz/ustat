#include <stddef.h>

int fmt_8longlong(char* s, unsigned long long val) {

    size_t l;
    unsigned long long v = val;

    for (l = 1; v > 9; l++) {
        v = v / 10;
    }

    if (s) {
        s += l;
        do {
            s--;
            *s = '0' + (val % 10);
            val /= 10;
        } while (val);
    }

    return l;
}

