#include "ustat.h"
#include "djb/open.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>

#if defined(BSD)
static int bsd_loadavg(double load[3]);
#else
static int linux_loadavg(const char* path, double load[3]);
#endif

static double _load[3] = {-1.0, -1.0, -1.0 };



int load_init(struct ustat_module* m, const char* s, size_t l) {
#if defined (BSD)
    bsd_loadavg(_load);
#else
    linux_loadavg("/proc/loadavg", _load);
#endif
    m->ready = 1;
    return 1;
}


int load_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int i;
    for (i = 0; i < 3; i++) {
        print_double(fd, _load[i], 2);
        if (i < 2) {
            write(fd, " ", 1);
        }
    }

    return 1;
}



#if defined(BSD)
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#ifndef VM_LOADAVG
#include <vm/vm_param.h>
#endif

static int bsd_loadavg(double load[3]) {

    struct loadavg l;
    size_t lsize = sizeof(l);
    int nelems =   sizeof(l.ldavg)/sizeof(fixpt_t);
    int            i;
    int            mib[2] = {CTL_VM, VM_LOADAVG};
    double         v;

    if (sysctl(mib, 2, &l, &lsize, 0, 0) != 0) {
        return 0;
    }

    load[0] = (double)l.ldavg[0] / (double)l.fscale;
    load[1] = (double)l.ldavg[1] / (double)l.fscale;
    load[2] = (double)l.ldavg[2] / (double)l.fscale;

    return 1;
}

#else

static int linux_loadavg(const char* path, double load[3]) {

    double v;
    int    p, i;
    int    fd = open_read(path);

    if (fd == -1) {
        close(fd);
        return 0;
    }

    for (i = 0; i < 3; i++) {

        p = scan_double_from_fd(fd, &v);
        if (p <= 0) {
            return 0;
        }
        load[i] = v;
    }

    close(fd);
    return 1;
}

#endif

