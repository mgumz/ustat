#include "ustat.h"
#include "djb/str.h"
#include <time.h>
#include <unistd.h>

int date_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    char buf[256];
    time_t t = time(0);
    struct tm* t2 = gmtime(&t);
    size_t n = strftime(buf, sizeof(buf), &s[str_len(m->name)], t2);
    if (n > 0) {
        write(fd, buf, n);
    }
    m->ready = 1;
    return 1;
}
