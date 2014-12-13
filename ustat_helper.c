#include "ustat.h"
#include "djb/fmt.h"

#include <sys/types.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>


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
