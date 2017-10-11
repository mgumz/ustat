#include "ustat.h"
#include "djb/fmt.h"
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/param.h>


static int _get_number_active_processes(size_t* nproc);

int nproc_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    size_t  nproc = 0;
    char*   buf;
    int     n;

    _get_number_active_processes(&nproc);

    n = fmt_ulong(0, nproc);
    buf = alloca(n);
    fmt_ulong(buf, nproc);
    write(fd, buf, n);

    return 1;
}


#if defined(BSD)

#include <sys/sysctl.h>
#include <sys/user.h> // struct kinfo_proc freebsd

static int _get_number_active_processes(size_t* nproc) {

    int           mib[] = {
        CTL_KERN,
        KERN_PROC,
#if defined (__APPLE__)
        KERN_PROC_ALL
#else
        KERN_PROC_PROC
#endif
    };
    const size_t  mibs = sizeof(mib)/sizeof(mib[0]);
    size_t        val;

    if (sysctl(mib, mibs, 0, &val, 0, 0) != 0) {
        return 0;
    }

    *nproc  = val / sizeof(struct kinfo_proc);

    return 1;
}

#else

// NOTES: sysinfo() yields strange results when called from
// inside a openvz-container. that's why we traverse /proc

#include <sys/types.h>
#include <dirent.h>
static int _get_number_active_processes(size_t* nproc) {

    size_t val = 0;
    struct dirent* entry;
    DIR* proc = opendir("/proc");

    if (!proc) {
        return 0;
    }

    // bold assumption: isdigit(entry->d_name[0]) is enough
    // to declare a process-id. alternative: scan_8long()
    for (entry = readdir(proc); entry; entry = readdir(proc)) {
        if (isdigit(entry->d_name[0])) {
            val++;
        }
    }

    closedir(proc);
    *nproc = val;

    return 1;
}

#endif

