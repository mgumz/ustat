#include "ustat.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>


static double _load[3] = {-1.0, -1.0, -1.0 };


static int _get_loadavg(double load[3]);

int load_init(struct ustat_module* m, const char* s, size_t l) {
    _get_loadavg(_load);
    m->ready = 1;
    return 1;
}

int load_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int i;
    for (i = 0; i < 3; i++) {
        write_double(fd, _load[i], 2);
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

static int _get_loadavg(double load[3]) {

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

#include <sys/sysinfo.h>

static int _get_loadavg(double load[3]) {

    double scale = (double)(1 << SI_LOAD_SHIFT);
    struct sysinfo si;

    if (sysinfo(&si) == -1) {
        return 0;
    }

    load[0] = ((double)si.loads[0]) / scale;
    load[1] = ((double)si.loads[1]) / scale;
    load[2] = ((double)si.loads[2]) / scale;

    return 1;
}


#endif

