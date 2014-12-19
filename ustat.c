// ustat - mikro-stat, tool to collect system-information
// and print it to stdout.

#include "ustat.h"
#include "djb/str.h"
#include <unistd.h>
#include <stdlib.h>

//
// modules
extern int pass_print(int, struct ustat_module*, const char*, size_t);
extern int load_init(struct ustat_module*, const char*, size_t);
extern int load_print(int, struct ustat_module*, const char*, size_t);
extern int date_print(int, struct ustat_module*, const char*, size_t);
extern int user_print(int, struct ustat_module*, const char*, size_t);
extern int uid_print(int, struct ustat_module*, const char*, size_t);
extern int nusers_init(struct ustat_module*, const char*, size_t);
extern int nusers_print(int, struct ustat_module*, const char*, size_t);
extern int mem_init(struct ustat_module*, const char*, size_t);
extern int mem_total_print(int, struct ustat_module*, const char*, size_t);
extern int mem_total_human_print(int, struct ustat_module*, const char*, size_t);
extern int mem_free_print(int, struct ustat_module*, const char*, size_t);
extern int mem_free_human_print(int, struct ustat_module*, const char*, size_t);
extern int mem_ratio_print(int, struct ustat_module*, const char*, size_t);

extern int nproc_print(int, struct ustat_module*, const char*, size_t);
extern int ncpus_print(int, struct ustat_module*, const char*, size_t);

extern int tcp_init(struct ustat_module*, const char*, size_t);
extern int ntcp_all_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp_closing_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp_established_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp_listen_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp6_all_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp6_closing_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp6_established_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp6_listen_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp4_all_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp4_closing_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp4_established_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp4_listen_print(int, struct ustat_module*, const char*, size_t);
extern int ntcp_print(int, struct ustat_module*, const char*, size_t);

extern int color_off(int, struct ustat_module*, const char*, size_t);
extern int color8_fg_normal_print(int, struct ustat_module*, const char*, size_t);
extern int color8_fg_bright_print(int, struct ustat_module*, const char*, size_t);
extern int color8_bg_normal_print(int, struct ustat_module*, const char*, size_t);
extern int color8_bg_bright_print(int, struct ustat_module*, const char*, size_t);
extern int xterm256_fg_print(int, struct ustat_module*, const char*, size_t);
extern int xterm256_bg_print(int, struct ustat_module*, const char*, size_t);
extern int rgb_fg_print(int, struct ustat_module*, const char*, size_t);
extern int rgb_bg_print(int, struct ustat_module*, const char*, size_t);



static int no_init(struct ustat_module* m, const char* s, size_t l) {
    return m->ready = 1;
}

static struct ustat_module modules[] = {
    // pass-thru, skip str_len(m->name) bytes
    {"",        0, 0, 0, no_init, pass_print },
    {"%",       0, 0, 0, no_init, pass_print },

    {"load",    0, 0, 0, load_init, load_print },
    {"date",    0, 0, 0, no_init, date_print },
    {"uid",     0, 0, 0, no_init, uid_print  },
    {"user",    0, 0, 0, no_init, user_print },
    {"nusers",  0, 0, 0, nusers_init, nusers_print },

    {"nproc",   0, 0, 0, no_init, nproc_print },
    {"ncpus",   0, 0, 0, no_init, ncpus_print },

    {"memfh",   0, 0, 0, mem_init, mem_free_human_print },
    {"memf",    0, 0, 0, mem_init, mem_free_print },
    {"memr",    0, 0, 0, mem_init, mem_ratio_print },
    {"memh",    0, 0, 0, mem_init, mem_total_human_print },
    {"mem",     0, 0, 0, mem_init, mem_total_print },

    {"tcpdump", 0, 0, 0, tcp_init, ntcp_print },
    {"tcp.e",   0, 0, 0, tcp_init, ntcp_established_print },
    {"tcp.l",   0, 0, 0, tcp_init, ntcp_listen_print },
    {"tcp.c",   0, 0, 0, tcp_init, ntcp_closing_print },
    {"tcp",     0, 0, 0, tcp_init, ntcp_all_print },
    {"tcp6.e",  0, 0, 0, tcp_init, ntcp6_established_print },
    {"tcp6.l",  0, 0, 0, tcp_init, ntcp6_listen_print },
    {"tcp6.c",  0, 0, 0, tcp_init, ntcp6_closing_print },
    {"tcp6",    0, 0, 0, tcp_init, ntcp6_all_print },
    {"tcp4.e",  0, 0, 0, tcp_init, ntcp4_established_print },
    {"tcp4.l",  0, 0, 0, tcp_init, ntcp4_listen_print },
    {"tcp4.c",  0, 0, 0, tcp_init, ntcp4_closing_print },
    {"tcp4",    0, 0, 0, tcp_init, ntcp4_all_print },

    // color-foo
    {"coff",   0, 0, 0, no_init, color_off },
    {"8#",     0, 0, 0, no_init, color8_fg_normal_print },
    {"8*",     0, 0, 0, no_init, color8_fg_bright_print },
    {"B#",     0, 0, 0, no_init, color8_bg_normal_print },
    {"B*",     0, 0, 0, no_init, color8_bg_bright_print },
    {"256#",   0, 0, 0, no_init, xterm256_fg_print },
    {"256*",   0, 0, 0, no_init, xterm256_bg_print },
    {"rgb#",   0, 0, 0, no_init, rgb_fg_print },
    {"rgb*",   0, 0, 0, no_init, rgb_bg_print },
};
static const size_t nmodules = sizeof(modules)/sizeof(struct ustat_module);

// usage: ustat "load.1" ":" "load.2" ":" "load.3"
int main(int argc, char* argv[]) {

    struct ustat_module** m = alloca(argc * sizeof(struct ustat_module*));

    // first, scan the given args and init() any module which is
    // not yet ready.
    int i, j;
    for (i = 1; i < argc; i++) {
        for (j = 1; j < nmodules; j++) {
            if (str_start(argv[i], modules[j].name)) {
                if (!modules[j].ready) {
                    modules[j].init(&modules[j], argv[i], str_len(argv[i]));
                }
                break;
            }
        }

        // any unrecognized argument is mapped to the
        // 'pass'-module (index 0).
        m[i] = &modules[j % nmodules];
    }

    // and .. print it
    for (i = 1; i < argc; i++) {
        m[i]->print(STDOUT_FILENO, m[i], argv[i], str_len(argv[i]));
    }

    return 0;
}

