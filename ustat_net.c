#include "ustat.h"
#include "djb/open.h"
#include "djb/byte.h"
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <netinet/tcp.h>

#if 0
enum {
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
    TCP_CLOSING,    /* Now a valid state */

    TCP_MAX_STATES  /* Leave at the end! */
};
#endif

const size_t TCP_MAX_STATES = TCP_CLOSING + 1;

// index 0 holds the sum
static uint64_t _tcp_counters[TCP_MAX_STATES];
static uint64_t _tcp4_counters[TCP_MAX_STATES];
static uint64_t _tcp6_counters[TCP_MAX_STATES];
const size_t _n_tcp_states = sizeof(_tcp_counters) / sizeof(_tcp_counters[0]);

static int process_proc_tcp(int fd, int skip_fields, uint64_t* store, size_t lstore);
static int print(int fd, uint64_t val);

static int init_tcp_stats();


int tcp_init(struct ustat_module* m, const char* s, size_t l) {

    static char inited = 0;
    int i;
    int fd;

    if (inited) {
        m->ready = 1;
        return 1;
    }

    if (init_tcp_stats() == 0) {
        return 0;
    }

    inited = 1;
    m->ready = 1;
    return 1;
}

int ntcp_established_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print(fd, _tcp_counters[TCP_ESTABLISHED]);
}

int ntcp_all_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print(fd, _tcp_counters[0]);
}

int ntcp_closing_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print(fd, _tcp_counters[TCP_CLOSING]);
}

int ntcp_listen_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print(fd, _tcp_counters[TCP_LISTEN]);
}


// internal functions
//

int static print(int fd, uint64_t val) {
    int n = fmt_8longlong(0, val);
    char* buf = alloca(n);
    fmt_8longlong(buf, val);
    write(fd, buf, n);
    return 1;
}

#if defined(BSD)
#include "ustat_net_bsd.c"
#else
#include "ustat_net_linux.c"
#endif
