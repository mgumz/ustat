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

extern int fs_init(struct ustat_module*, const char*, size_t);
extern int fs_total_human_print(int, struct ustat_module*, const char*, size_t);
extern int fs_total_print(int, struct ustat_module*, const char*, size_t);
extern int fs_ratio_print(int, struct ustat_module*, const char*, size_t);
extern int fs_free_human_print(int, struct ustat_module*, const char*, size_t);
extern int fs_free_print(int, struct ustat_module*, const char*, size_t);
extern int fs_used_human_print(int, struct ustat_module*, const char*, size_t);
extern int fs_used_print(int, struct ustat_module*, const char*, size_t);

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
    {"",        "",                       0, 0, 0, no_init, pass_print },
    {"%",       "pass through",           0, 0, 0, no_init, pass_print },
 
    {"load",    "system load",            0, 0, 0, load_init, load_print },
    {"date",    "date, strftime",         0, 0, 0, no_init, date_print },
    {"uid",     "render user id",         0, 0, 0, no_init, uid_print  },
    {"user",    "render user name",       0, 0, 0, no_init, user_print },
    {"nusers",  "number of active users", 0, 0, 0, nusers_init, nusers_print },

    {"nproc",   "number of processes",    0, 0, 0, no_init, nproc_print },
    {"ncpus",   "number of cpus",         0, 0, 0, no_init, ncpus_print },

    {"memfh",   "memory free, human",     0, 0, 0, mem_init, mem_free_human_print },
    {"memf",    "memory free",            0, 0, 0, mem_init, mem_free_print },
    {"memr",    "memory free, ratio",     0, 0, 0, mem_init, mem_ratio_print },
    {"memh",    "memory total, human",    0, 0, 0, mem_init, mem_total_human_print },
    {"mem",     "memory total",           0, 0, 0, mem_init, mem_total_print },

    {"fsth",    "filesystem total, human",0, 0, 0, fs_init,  fs_total_human_print },
    {"fst",     "filesystem total",       0, 0, 0, fs_init,  fs_total_print },
    {"fsr",     "filesystem ratio",       0, 0, 0, fs_init,  fs_ratio_print },
    {"fsfh",    "filesystem free, human", 0, 0, 0, fs_init,  fs_free_human_print },
    {"fsf",     "filesystem free",        0, 0, 0, fs_init,  fs_free_print },
    {"fsuh",    "filesystem used, human", 0, 0, 0, fs_init,  fs_used_human_print },
    {"fsu",     "filesystem used",        0, 0, 0, fs_init,  fs_used_print },

    {"tcp6.e",  "tcp6 established",       0, 0, 0, tcp_init, ntcp6_established_print },
    {"tcp6.l",  "tcp6 listening",         0, 0, 0, tcp_init, ntcp6_listen_print },
    {"tcp6.c",  "tcp6 closing",           0, 0, 0, tcp_init, ntcp6_closing_print },
    {"tcp6",    "tcp6 total",             0, 0, 0, tcp_init, ntcp6_all_print },
    {"tcp4.e",  "tcp4 established",       0, 0, 0, tcp_init, ntcp4_established_print },
    {"tcp4.l",  "tcp4 listening",         0, 0, 0, tcp_init, ntcp4_listen_print },
    {"tcp4.c",  "tcp4 closing",           0, 0, 0, tcp_init, ntcp4_closing_print },
    {"tcp4",    "tcp4 total",             0, 0, 0, tcp_init, ntcp4_all_print },
    {"tcpdump", "tcpdump",                0, 0, 0, tcp_init, ntcp_print },
    {"tcp.e",   "tcp established",        0, 0, 0, tcp_init, ntcp_established_print },
    {"tcp.l",   "tcp listening",          0, 0, 0, tcp_init, ntcp_listen_print },
    {"tcp.c",   "tcp closing",            0, 0, 0, tcp_init, ntcp_closing_print },
    {"tcp",     "tcp total",              0, 0, 0, tcp_init, ntcp_all_print },

    // color-foo
    {"coff",   "color off",               0, 0, 0, no_init, color_off },
    {"8#",     "fg-color, normal",        0, 0, 0, no_init, color8_fg_normal_print },
    {"8*",     "fg-color, bright",        0, 0, 0, no_init, color8_fg_bright_print },
    {"B#",     "bg-color, normal",        0, 0, 0, no_init, color8_bg_normal_print },
    {"B*",     "bg-color, bright",        0, 0, 0, no_init, color8_bg_bright_print },
    {"256#",   "fg-color, 256colors",     0, 0, 0, no_init, xterm256_fg_print },
    {"256*",   "bg-color, 256colors",     0, 0, 0, no_init, xterm256_bg_print },
    {"rgb#",   "fg-color, rgb",           0, 0, 0, no_init, rgb_fg_print },
    {"rgb*",   "bg-color, rgb",           0, 0, 0, no_init, rgb_bg_print },
};
static const size_t nmodules = sizeof(modules)/sizeof(struct ustat_module);

int usage(void);

// usage: ustat "load.1" ":" "load.2" ":" "load.3"
int main(int argc, char* argv[]) {

    //struct ustat_module** m = alloca(argc * sizeof(struct ustat_module*));
    struct ustat_module* m[argc];
    int i, j;

    if (argc == 2 && str_diffn(argv[1], "-h", 2) == 0) {
        return usage();
    }

    // first, scan the given args and init() any module which is
    // not yet ready.
    for (i = 1; i < argc; i++) {
        for (j = 1; j < nmodules; j++) {
            if (str_start(argv[i], modules[j].name)) {
                if (!modules[j].ready) {
                    int rc = modules[j].init(&modules[j], argv[i], str_len(argv[i]));
                    if (rc == 0) {
                        const char error_cant_init_module[] = "\nerror init module \"";
                        write(STDERR_FILENO, error_cant_init_module, sizeof(error_cant_init_module));
                        write(STDERR_FILENO, modules[j].name, str_len(modules[j].name));
                        os_exit(1, "\"");
                    }
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

int usage() {
    int i;
    for (i = 1; i < nmodules; i++) {
        write(STDOUT_FILENO, modules[i].name, str_len(modules[i].name));
        write(STDOUT_FILENO, "\t", 1);
        write(STDOUT_FILENO, modules[i].descr, str_len(modules[i].descr));
        write(STDOUT_FILENO, "\n", 1);
    }
    return 0;
}

