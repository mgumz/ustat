#include "ustat.h"
#include "djb/open.h"
#include "djb/fmt.h"

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

    char buf[6];
    int n;
    int i;
    double v;

    for (i = 0; i < 3; i++) {

        v = _load[i];

        // sign
        if (v < 0) {
            write(fd, "-", 1);
            v *= -1.0;
        }

        // decimal
        n = fmt_ulong(buf, (unsigned long)v);
        write(fd, buf, n);

        // fraction
        write(fd, ".", 1);

        v = v * 100.0;
        n = fmt_ulong(buf, ((unsigned long)v) % 100);
        if (n < 2) {
            write(fd, "0", 1);
        }
        write(fd, buf, n);

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
#include <vm/vm_param.h>

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
