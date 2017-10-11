#include "../ustat.h"
#include "../djb/fmt.h"
#include <unistd.h>

int write_8longlong(int fd, int64_t val) {
    char buf[32];
    int n = fmt_8longlong(buf, val);
    write(fd, buf, n);
    return 1;
}

