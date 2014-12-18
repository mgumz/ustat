
static void _sum_counters(uint64_t* to, uint64_t* from, size_t l);

//
// fallback: in case we are not able to retrieve the socket stats
// via the netlink-inet_diag interface we have to scan /proc/net/tcp
// and /proc/net/tcp6 ourselfs
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

static int _read_byte(int fd, struct buffered_byte_reader* br) {

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

static int _process_proc_tcp(int fd, int state_field, uint64_t* store, size_t lstore) {

    int rc = 0;
    struct buffered_byte_reader r = {.pos = 0, .end = 0};
    unsigned long val;

    // skip first line by treating it as any other line behind the 'state'
    // field
seek_to_eol:
    for ( ; _read_byte(fd, &r); ) {
        if (r.err) {
            goto end;
        }
        if (r.cur == '\n') {
            goto skip_leading_ws;
        }
    }
    goto end;

skip_leading_ws:
    for ( ; _read_byte(fd, &r) ; ) {
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
    for ( ; r.field < state_field && _read_byte(fd, &r); ) {
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
    if (!_read_byte(fd, &r)) {
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



// the should-always-work method. maybe fast enough, but it takes ~0.2s to parse a 1mb 
// /proc/net/tcp file with ~5k entries on a openvz-guest. it's noticable.
static int init_tcp_stats_via_proc_net_tcp() {

    static const char tcpv4_name[] = "/proc/net/tcp";
    static const char tcpv6_name[] = "/proc/net/tcp6";

    int          i;
    int          fd = open_read(tcpv4_name);

    if (fd < 0) { // not on linux? huh?
        return 0;
    }

    byte_zero(_tcp_counters, sizeof(_tcp_counters));
    byte_zero(_tcp4_counters, sizeof(_tcp4_counters));
    byte_zero(_tcp6_counters, sizeof(_tcp6_counters));

    // tcpv4
    _process_proc_tcp(fd, 3, _tcp4_counters, _n_tcp_states);
    close(fd);

    _sum_counters(_tcp_counters, _tcp4_counters, _n_tcp_states);

    // tcpv6, if available
    fd = open_read(tcpv6_name);
    if (fd > 0) {
        _process_proc_tcp(fd, 3, _tcp6_counters, _n_tcp_states);
        close(fd);
        _sum_counters(_tcp_counters, _tcp6_counters, _n_tcp_states);
    }

    return 1;
}


// maybe a faster approach to get the stats of the tcp-sockets: netlink to the
// kernel.
//
// good sources of information:
//
// * http://kristrev.github.io/2013/07/26/passive-monitoring-of-sockets-on-linux/
// * https://github.com/kristrev/inet-diag-example
// * http://www.linuxjournal.com/article/7356

#if USTAT_NETLINK

static int _request_tcp_diag(int sock, int family);
static int64_t _count_tcp_states(int sock, uint8_t* buf, size_t l, uint64_t* counter);

static const char error_no_netlink_to_kernel[] = "can't get netlink-socket from kernel\n";
static const char error_no_tcp_diag[] = "can't get tcp-diag via netlink\n";

static int init_tcp_stats_via_netlink() {

    char                    done = 0;
    int                     sock = 0;
    uint8_t                 buf[4096]; // TODO: use page-size here
    uint64_t                rc = 0;

    if((sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_INET_DIAG)) == -1){
        write(STDERR_FILENO, error_no_netlink_to_kernel, sizeof(error_no_netlink_to_kernel));
        return 0;
    }

    // try ipv4
    if(_request_tcp_diag(sock, AF_INET) < 0){
        write(STDERR_FILENO, error_no_tcp_diag, sizeof(error_no_tcp_diag));
        return 0;
    }

    if (_count_tcp_states(sock, buf, sizeof(buf), _tcp4_counters) >= 0) {
        _sum_counters(_tcp_counters, _tcp4_counters, _n_tcp_states);
        rc = 1;
        if(_request_tcp_diag(sock, AF_INET6) >= 0) {
            _count_tcp_states(sock, buf, sizeof(buf), _tcp6_counters);
            _sum_counters(_tcp_counters, _tcp6_counters, _n_tcp_states);
        }
    }
    close(sock);

    return rc;
}

static int64_t _count_tcp_states(int sock, uint8_t* buf, size_t l, uint64_t* counter) {

    struct inet_diag_msg* msg;
    const size_t          msg_size = sizeof(*msg);
    char                  done;
    int64_t               i = 0;

    for (done = 0; !done ; ) {

        int              n = recv(sock, buf, l, 0);
        struct nlmsghdr* hdr = (struct nlmsghdr*)buf;

        for ( ;NLMSG_OK(hdr, n); ) {
            if (hdr->nlmsg_type == NLMSG_DONE) {
                done = 1;
                break;
            } else if (hdr->nlmsg_type == NLMSG_ERROR) {
                return -1;
            }

            msg = (struct inet_diag_msg*)NLMSG_DATA(hdr);

            i++;
            counter[msg->idiag_state]++;
            hdr = NLMSG_NEXT(hdr, n);
        }
    }

    return i;
}


static int _request_tcp_diag(int sock, int family) {

    struct msghdr           msg;
    struct nlmsghdr         nlh;
    struct inet_diag_req_v2 c_req;
    struct sockaddr_nl      sa;
    struct rtattr           rta;
    struct iovec            iov[4];

    byte_zero(&msg, sizeof(msg));
    byte_zero(&nlh, sizeof(nlh));
    byte_zero(&sa, sizeof(sa));
    byte_zero(&c_req, sizeof(c_req));

    sa.nl_family = AF_NETLINK;
    c_req.sdiag_family = family;
    c_req.sdiag_protocol = IPPROTO_TCP;
    c_req.idiag_states = 0xfff; // 'TCPF_ALL'

    nlh.nlmsg_len = NLMSG_LENGTH(sizeof(c_req));
    nlh.nlmsg_flags = NLM_F_DUMP|NLM_F_REQUEST;
    nlh.nlmsg_type = SOCK_DIAG_BY_FAMILY;

    iov[0].iov_base = (void*)&nlh;
    iov[0].iov_len = sizeof(nlh);
    iov[1].iov_base = (void*)&c_req;
    iov[1].iov_len = sizeof(c_req);

    msg.msg_name = (void*)&sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;

    return sendmsg(sock, &msg, 0);
}

#endif // USTAT_NETLINK

// sum all elements from 'from[1-(l-1)]' into 'from[0] and
// add 'from[i]' onto 'to[i]'
static void _sum_counters(uint64_t* to, uint64_t* from, size_t l) {
    size_t i;
    for (i = 1; i < l; i++) {
        from[0] += from[i];
        to[i] += from[i];
    }
    to[0] += from[0];
}


static int init_tcp_stats() {

    int rc = 0;
#if USTAT_NETLINK
    if (!getenv("FORCE_PROC_NET_TCP")) {
        rc = init_tcp_stats_via_netlink();
    }
#endif
    if (!rc) {
        rc = init_tcp_stats_via_proc_net_tcp();
    }

    return rc;
}

