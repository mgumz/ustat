#include "djb/str.h"
#include "ustat.h"

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static const uint64_t MICRO_SECS = 1000L * 1000L;

int date_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    char buf[256];
    struct timeval t;
    struct tm* t2;
    size_t n;

    gettimeofday(&t, 0);
    t.tv_sec = ((t.tv_sec * MICRO_SECS) + t.tv_usec) / MICRO_SECS;
    t2 = gmtime(&t.tv_sec);
    n = strftime(buf, sizeof(buf), &s[str_len(m->name)], t2);

    if (n > 0) {
        write(fd, buf, n);
    }
    m->ready = 1;
    return 1;
}
