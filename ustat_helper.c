#include "ustat.h"
#include "djb/fmt.h"

#include <stdint.h>
#include <float.h>
#include <math.h>
#include <unistd.h>


int no_init(struct ustat_module* m, const char* s, size_t l) {
    return m->ready = 1;
}

int print_double(int fd, double val, int prec) {

    // TODO: rounding

    int n, s;
    char buf[20]; // 19 to hold uint64_t

    if (val < 0) {
        write(fd, "-", 1);
    }

    n = fmt_ulong(buf, (uint64_t)val);
    write(fd, buf, n);

    if (prec == 0) {
        return 1;
    }

    s = 1;
    for (n = 0; n < prec; n++) {
        s *= 10;
    }

    val *= (double)s;
    n = fmt_ulong(buf, ((uint64_t)val) % s);
    s = prec;

    write(fd, ".", 1);
    for ( ; (s - n) > 0; s--) {
        write(fd, "0", 1);
    }
    write(fd, buf, n);

    return 1;
}


int scan_hex(const char* s, size_t l, unsigned long* n) {

    char c;
    size_t i;
    for (i = 0 ;i < l; i++) {

        // '0 - '9'
        c = s[i] - '0';
        if (c < 0) {
            return 0;
        } else if (c <= 9) {
            goto add;
        }

        // 'A' - 'F'
        c = s[i] - 'A';
        if (c < 0) {
            return 0;
        } else if (c <= 5) {
            c += 10;
            goto add;
        }

        // 'a' - 'f'
        c = s[i] - 'a';
        if (c < 0 || c > 5) {
            return 0;
        }
        c += 10;
add:
        if (n) {
            *n = (*n << 4) + c;
        }
    }
    return i;
}


// copy the bytes from 'in_fd' to 'out_fd', stop after finding
// 'n' chars 'sep'
int first_n_fields(int out_fd, int in_fd, char sep, int n) {

    int r;
    char c;

    for ( ;; ) {

        r = read(in_fd, &c, 1);
        if (r <= 0) {
            break;
        }

        if (c == sep) {
            n--;
        }

        if (n <= 0) {
            break;
        }

        write(out_fd, &c, 1);
    }

    return 1;
}

// http://www.netlib.org/fp/dtoa.c is the correct one, this is just
// a very very very simple approach.
//
// only form accepted: [-]dec[.frac]
int scan_double_from_fd(int fd, double* val) {

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
    v = (double)n * (1.0 / (double)l);

finish:
    v *= (double)sign;
    if (val) {
        *val = v;
    }

    return pos;
}

