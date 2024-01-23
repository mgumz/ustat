#include "../ustat.h"
#include "../djb/fmt.h"
#include <unistd.h>

int write_uint64(int fd, uint64_t val) {
    char buf[FMT_ULONG];
    int n = fmt_uint64(buf, val);
    write(fd, buf, n);
    return 1;
}

