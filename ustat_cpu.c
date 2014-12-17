#include "ustat.h"
#include "djb/fmt.h"
#include <unistd.h>

int ncpus_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int n;
    char buf[3];
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus < 0) {
        return 0;
    }

    n = fmt_ulong(buf, ncpus);
    write(fd, buf, n);

    return 1;
}
