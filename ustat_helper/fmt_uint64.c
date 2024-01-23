#include <stddef.h>
#include <stdint.h>

int fmt_uint64(char* s, uint64_t val) {

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

