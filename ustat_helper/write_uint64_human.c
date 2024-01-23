#include "../ustat.h"
#include <unistd.h>

int write_double(int fd, double val, int prec);

int write_uint64_human(int fd, uint64_t min, uint64_t val) {

    static const char  human_suf[] = "_kmgt";
    static const char* last_suf = (human_suf + sizeof(human_suf));
    const char*        suf = human_suf;
    static const unsigned long long base = 1000;
    unsigned long long v = val;
    unsigned long long scale = 1;

    for ( ; ((v / base) > min) && (suf < last_suf+1); ) {
        v = v / base;
        scale *= base;
        suf++;
    }

    write_double(fd, (double)val/(double)scale, 2);
    if (suf > human_suf) {
        write(fd, suf, 1);
    }

    return 1;
}


