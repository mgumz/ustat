#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

// http://www.netlib.org/fp/dtoa.c is the correct one, this is just
// a very very very simple approach.
//
// only form accepted: [-]dec[.frac]
int read_double_from_fd(int fd, double* val) {

    size_t pos = 0;
    double v = 0.0;
    char c = 0;
    char sign = 1;
    uint64_t n = 0;
    uint64_t l = 1;

    // skip leading spaces
    pos += read(fd, &c, 1);

    for ( ; pos > 0 && (c == ' ' || c == '\t'); ) {
        pos += read(fd, &c, 1);
    }

    // sign or eof
    if (c == '-') {
        sign = -1;
        pos += read(fd, &c, 1);
    }

    if (pos <= 0){
        goto finish;
    }

    // decimal
    for ( ; c >= 0 ; ) {
        if (c >= '0' && c <= '9') {
            n = (n * 10) + (c - '0');
        } else if (c == '.') {
            pos += read(fd, &c, 1);
            break;
        } else {
            return pos;
        }
        pos += read(fd, &c, 1);
    }
    v = (double)n;

    // fraction
    for (n = 0; c >= 0; ) {
        if (c >= '0' && c <= '9') {
            l *= 10;
            n = (n * 10) + (c - '0');
        } else {
            break;
        }
        pos += read(fd, &c, 1);
    }
    v += (double)n * (1.0 / (double)l);

finish:
    v *= (double)sign;
    if (val) {
        *val = v;
    }

    return pos;
}

