#include "ustat.h"
#include "djb/open.h"
#include "djb/byte.h"
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>

#include <sys/param.h>


const size_t    _n_tcp_states = 12; // 11 + 1 to sum it up ('total')
static uint64_t _tcp_counters[_n_tcp_states];
static uint64_t _tcp4_counters[_n_tcp_states];
static uint64_t _tcp6_counters[_n_tcp_states];


static int _print(int fd, uint64_t val);
static int _init_tcp_stats();
static void _sum_counters(uint64_t* to, uint64_t* from);


#if defined (BSD)

#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_var.h>
#include <errno.h>

enum {
    TCP_CLOSE =         TCPS_CLOSED,
    TCP_LISTEN =        TCPS_LISTEN,
    TCP_SYN_SENT =      TCPS_SYN_SENT,
    TCP_SYN_RECV =      TCPS_SYN_RECEIVED,
    TCP_ESTABLISHED =   TCPS_ESTABLISHED,
    TCP_CLOSE_WAIT =    TCPS_CLOSE_WAIT,
    TCP_FIN_WAIT1 =     TCPS_FIN_WAIT_1,
    TCP_CLOSING =       TCPS_CLOSING,
    TCP_LAST_ACK =      TCPS_LAST_ACK,
    TCP_FIN_WAIT2 =     TCPS_FIN_WAIT_2,
    TCP_TIME_WAIT =     TCPS_TIME_WAIT,

    TCP_STATE_FIRST =   0,
    TCP_STATE_LAST =    TCP_TIME_WAIT,
    INDEX_TOTAL =       TCP_STATE_LAST + 1
};

#else

#ifdef __dietlibc__
typedef long __kernel_long_t;
typedef unsigned long __kernel_ulong_t;
typedef unsigned short __kernel_sa_family_t;
#endif

#include <netinet/in.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/inet_diag.h>

#if defined(USTAT_NETLINK) && USTAT_NETLINK
// include it here because if inet_diag_req_v2 is not defined it's
// still an older kernel where <linux/sock_diag.h> is not available
#  include <linux/sock_diag.h>
#endif // USTAT_NETLINK

enum{
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING

    TCP_STATE_FIRST = TCP_ESTABLISHED,
    TCP_STATE_LAST = TCP_CLOSING,
    INDEX_TOTAL = 0
};

#endif

int tcp_init(struct ustat_module* m, const char* s, size_t l) {

    static char inited = 0;
    int i;
    int fd;

    if (inited) {
        m->ready = 1;
        return 1;
    }

    if (_init_tcp_stats() == 0) {
        return 0;
    }

    inited = 1;
    m->ready = 1;
    return 1;
}

int ntcp_established_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp_counters[TCP_ESTABLISHED]);
}

int ntcp_all_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp_counters[INDEX_TOTAL]);
}

int ntcp_closing_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp_counters[TCP_CLOSING]);
}

int ntcp_listen_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp_counters[TCP_LISTEN]);
}

int ntcp6_established_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp6_counters[TCP_ESTABLISHED]);
}

int ntcp6_all_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp6_counters[INDEX_TOTAL]);
}

int ntcp6_closing_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp6_counters[TCP_CLOSING]);
}

int ntcp6_listen_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp6_counters[TCP_LISTEN]);
}

int ntcp4_established_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp4_counters[TCP_ESTABLISHED]);
}

int ntcp4_all_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp4_counters[INDEX_TOTAL]);
}

int ntcp4_closing_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp4_counters[TCP_CLOSING]);
}

int ntcp4_listen_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return _print(fd, _tcp4_counters[TCP_LISTEN]);
}

int ntcp_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int i;
    _print(fd, INDEX_TOTAL);
    write(fd, ":", 1);
    for (i = 0; i < _n_tcp_states; i++) {
        _print(fd, _tcp_counters[i]);
        write(fd, ".", 1);
        _print(fd, _tcp4_counters[i]);
        write(fd, ".", 1);
        _print(fd, _tcp6_counters[i]);
        write(fd, " ", 1);
    }
    return 1;
}


// internal functions
//

int static _print(int fd, uint64_t val) {
    int n = fmt_8longlong(0, val);
    char* buf = alloca(n);
    fmt_8longlong(buf, val);
    write(fd, buf, n);
    return 1;
}

// sum all elements from 'from[1-(l-1)]' into 'from[0] and
// add 'from[i]' onto 'to[i]'
static void _sum_counters(uint64_t* to, uint64_t* from) {
    size_t i;
    for (i = TCP_STATE_FIRST; i < TCP_STATE_LAST+1; i++) {
        from[INDEX_TOTAL] += from[i];
        to[i] += from[i];
    }
    to[INDEX_TOTAL] += from[INDEX_TOTAL];
}



#if defined(BSD)
#include "ustat_tcp_bsd.c"
#else
#include "ustat_tcp_linux.c"
#endif

