#include "ustat.h"
#include "djb/fmt.h"
#include <stdlib.h>
#include <unistd.h>

int ncpus_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int n;
    char* buf;
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus < 0) {
        return 0;
    }

    n = fmt_ulong(0, ncpus);
    buf = alloca(n);
    n = fmt_ulong(buf, ncpus);
    write(fd, buf, n);

    return 1;
}
