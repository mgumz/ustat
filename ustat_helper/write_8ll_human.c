#include "../ustat.h"
#include <unistd.h>

int write_double(int fd, double val, int prec);

int write_8longlong_human(int fd, unsigned long long min, unsigned long long val) {

    static const char  human_suf[] = "_kmgt";
    static const char* last_suf = (human_suf + sizeof(human_suf));
    const char*        suf = human_suf;
    unsigned long long v = val;
    unsigned long long scale = 1;

    for ( ; ((v/1000) > min) && (suf < last_suf+1); ) {
        v = v / 1000;
        scale *= 1000;
        suf++;
    }

    write_double(fd, (double)val/(double)scale, 2);
    if (suf > human_suf) {
        write(fd, suf, 1);
    }

    return 1;
}


