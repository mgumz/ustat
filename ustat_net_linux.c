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


// maybe enough, but it takes ~0.2s to parse a 1mb /proc/net/tcp file
// on a openvz-guest. it's noticable.
static int init_tcp_stats_via_proc_net_tcp() {

    int i;
    int fd = open_read("/proc/net/tcp");
    if (fd < 0) {
        return 0;
    }

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


// maybe a faster approach to get the stats of the tcp-sockets: netlink to the
// kernel.
//
// good sources of information:
//
// * http://kristrev.github.io/2013/07/26/passive-monitoring-of-sockets-on-linux/
// * https://github.com/kristrev/inet-diag-example
// * http://www.linuxjournal.com/article/7356

#include <errno.h>
#include <string.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/inet_diag.h>

static int send_diag_msg(int sock);

static int init_tcp_stats_via_netlink() {

    char done = 0;
    int nl_sock = 0;
    struct inet_diag_msg* msg;
    const size_t msg_size = sizeof(*msg);
    uint8_t buf[4096]; // TODO: use page-size here

    if((nl_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_INET_DIAG)) == -1){
        os_exit(1, strerror(errno));
    }

    if(send_diag_msg(nl_sock) < 0){
        os_exit(1, strerror(errno));
    }

    for ( ; !done ; ) {

        int n = recv(nl_sock, buf, sizeof(buf), 0);
        int rtalen = 0;
        struct nlmsghdr* nl_header = (struct nlmsghdr*)buf;

        for ( ;NLMSG_OK(nl_header, n); ) {
            if (nl_header->nlmsg_type == NLMSG_DONE) {
                done = 1;
                break;
            } else if (nl_header->nlmsg_type == NLMSG_ERROR) {
                done = 1;
                os_exit(2, "error in reading nlmsgs");
            }

            msg = (struct inet_diag_msg*)NLMSG_DATA(nl_header);
            rtalen = nl_header->nlmsg_len - NLMSG_LENGTH(msg_size);
            // TODO parsing the msg
            nl_header = NLMSG_NEXT(nl_header, n);
        }
    }

    return 1;
}


static int send_diag_msg(int sock) {

    struct msghdr           msg;
    struct nlmsghdr         nlh;
    struct inet_diag_req    c_req; // kernel 2.6, inet_diag_req_v2 since v3.3
    struct sockaddr_nl      sa;
    struct rtattr           rta;
    struct iovec            iov[4];

    byte_zero(&msg, sizeof(msg));
    byte_zero(&nlh, sizeof(nlh));
    byte_zero(&sa, sizeof(sa));
    byte_zero(&c_req, sizeof(c_req));

    sa.nl_family = AF_NETLINK;
    c_req.idiag_family = AF_INET;
    c_req.idiag_states = TCPF_ALL;
    //c_req.idiag_protocol = IPROTO_TCP;
    //c_req.idiag_states = TCPF_ALL;

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



static int init_tcp_stats() {
    return init_tcp_stats_via_netlink();
}

