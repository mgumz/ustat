#include "ustat.h"
#include "djb/fmt.h"
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/param.h>

#include <sys/sysctl.h>

int nproc_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int     mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
    size_t  nproc;
    char*   buf;
    int     n;

    if (sysctl(mib, sizeof(mib)/sizeof(mib[0]), 0, &nproc, 0, 0) != 0) {
        return 0;
    }

    nproc = nproc / sizeof(struct kinfo_proc);
    n = fmt_ulong(0, nproc);
    buf = alloca(n);
    fmt_ulong(buf, nproc);
    write(fd, buf, n);

    return 1;
}
