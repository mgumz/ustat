//
// * http://src.gnu-darwin.org/src/usr.bin/sockstat/sockstat.c.html
//

#include "ustat.h"

#include <sys/sysctl.h>
#include <stdlib.h>
#include <unistd.h>

static int _init_tcp_stats() {

    static const char name[] = "net.inet.tcp.pcblist";

    int rc = 0;
    void* buf = 0;
    size_t l;
    struct xinpgen* xig;
    struct xinpgen* end_xig;
    struct xtcpcb* xtp;

    // allocate as much ram as sysctl demands
    if (sysctlbyname(name, 0, &l, 0, 0) != 0) {
        return 0;
    }

    buf = malloc(l);

    if (sysctlbyname(name, buf, &l, 0, 0) != 0) {
        goto out;
    }

    // check if what we got is complete
    xig = (struct xinpgen*)buf;
    end_xig = (struct xinpgen*)(void*)((char*)buf + l - sizeof(*end_xig));

    if (xig->xig_len != sizeof(*xig) || end_xig->xig_len != sizeof(*end_xig)) {
        os_exit(2, "struct xinpgen size mismatch\n");
    }

    // xig->xig_gen != exig->xig_gen -> inconsitent data

    for (;;) {

        xig = (struct xinpgen*)(void*)((char*)xig + xig->xig_len);
        if (xig >= end_xig) {
            break;
        }

        xtp = (struct xtcpcb*)xig;
        if (xtp->xt_len != sizeof(*xtp)) {
            // mismatch of size
            goto out;
        }

        size_t i = 0;
#if __FreeBSD_version >= 1200026
        i = xtp->t_state;
#else
        i = xtp->xt_tp.t_state;
#endif

        if (xtp->xt_inp.inp_vflag & INP_IPV4) {
            _tcp4_counters[i]++;
        } else if (xtp->xt_inp.inp_vflag & INP_IPV6) {
            _tcp6_counters[i]++;
        }

        rc = 1;
    }

out:
    free(buf);

    if (rc) {
        _sum_counters(_tcp_counters, _tcp4_counters);
        _sum_counters(_tcp_counters, _tcp6_counters);
    }

    return rc;
}
