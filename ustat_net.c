#include "ustat.h"
#include "djb/open.h"
#include "djb/byte.h"
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>

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


#else

#include <ctype.h>

//
// collecting tcp-connection stats under linux is a matter of parsing
// /proc/net/tcp and /proc/net/tcp6
//
// 'ss' (iproute2) and 'netstat' do it similar

struct buffered_byte_reader  {
    char    prev;
    char    cur;

    int     field;
    int     pos; // position in buf
    int     end; // end-position in buf
    size_t  consumed;
    ssize_t err;

    // 16k buffer. why? because /proc/net/tcp get's quite 'huge', for moderate
    // traffic it's in the range of 'megabyte'. parsing that file in 16k
    // blocks yields 'best' speed vs use or ram
    char    buf[16 * 1024];
};

static int read_byte(int fd, struct buffered_byte_reader* br) {

    br->pos++;

    if (br->pos >= br->end) { // fill buffer when needed
        br->end = read(fd, br->buf, sizeof(br->buf));
        br->pos = 0;
    }

    if (br->end > 0) {
        br->consumed++;
        br->cur = br->buf[br->pos];
        return 1;
    }

    br->err = br->end;
    return 0;
}

static int process_proc_tcp(int fd, int state_field, uint64_t* store, size_t lstore) {

    int rc = 0;
    struct buffered_byte_reader r = {.pos = 0, .end = 0};
    unsigned long val;

    // skip first line by treating it as any other line behind the 'state'
    // field
seek_to_eol:
    for ( ; read_byte(fd, &r); ) {
        if (r.err) {
            goto end;
        }
        if (r.cur == '\n') {
            goto skip_leading_ws;
        }
    }
    goto end;

skip_leading_ws:
    for ( ; read_byte(fd, &r) ; ) {
        if (r.err) {
            goto end;
        }
        if (!isspace(r.cur)) {
            goto skip_n_fields;
        }
    }
    goto end;

skip_n_fields:
    r.field = 0;
    r.prev = r.cur;
    for ( ; r.field < state_field && read_byte(fd, &r); ) {
        if (r.err) {
            goto end;
        }
        if (isspace(r.prev) && !isspace(r.cur)) {
            r.field++;
        }
        r.prev = r.cur;
    }

    // we read to few fields
    if (r.field != state_field) {
        goto end;
    }

    // m->prev now has the first byte of the field we are interested in,
    // read one more
    if (!read_byte(fd, &r)) {
        goto end;
    }

    val = 0;
    // .prev and .cur are next to each other
    if (scan_hex(&r.prev, 2, &val) != 2) {
        goto end;
    }

    // we extracted a value which does not fit into 'store'
    if (val > lstore) {
        goto end;
    }

    rc = 1;
    store[val]++;

    goto seek_to_eol;

end:

    return rc;
}


static int init_tcp_stats() {

    int i;
    int fd = open_read("/proc/net/tcp");
    if (fd < 0) {
        return 0;
    }

    inited = 1;

    byte_zero(_tcp_counters, sizeof(_tcp_counters));
    byte_zero(_tcp4_counters, sizeof(_tcp4_counters));
    byte_zero(_tcp6_counters, sizeof(_tcp6_counters));

    // tcpv4
    process_proc_tcp(fd, 3, _tcp4_counters, _n_tcp_states);
    close(fd);

    for (i = 1; i < _n_tcp_states; i++) {
        _tcp4_counters[0] += _tcp4_counters[i];
        _tcp_counters[i] += _tcp4_counters[i];
    }
    _tcp_counters[0] = _tcp4_counters[0];

    // tcpv6, if available
    fd = open_read("/proc/net/tcp6");
    if (fd > 0) {
        process_proc_tcp(fd, 3, _tcp6_counters, _n_tcp_states);
        close(fd);
        for (i = 1; i < _n_tcp_states; i++) {
            _tcp6_counters[0] += _tcp6_counters[i];
            _tcp_counters[i] += _tcp6_counters[i];
        }
        _tcp_counters[0] += _tcp6_counters[0];
    }

    return 1;
}

#endif

