// ustat - mikro-stat, tool to collect system-information
// and print it to stdout.

#include "ustat.h"
#include "djb/str.h"
#include <unistd.h>
#include <stdlib.h>

//
// modules
extern int pass_print(int, struct ustat_module*, const char*, size_t);
extern int load_print(int, struct ustat_module*, const char*, size_t);
extern int date_print(int, struct ustat_module*, const char*, size_t);
extern int user_print(int, struct ustat_module*, const char*, size_t);
extern int uid_print(int, struct ustat_module*, const char*, size_t);
extern int nusers_init(struct ustat_module*, const char*, size_t);
extern int nusers_print(int, struct ustat_module*, const char*, size_t);

extern int color_off(int, struct ustat_module*, const char*, size_t);
extern int color8_fg_normal_print(int, struct ustat_module*, const char*, size_t);
extern int color8_fg_bright_print(int, struct ustat_module*, const char*, size_t);
extern int color8_bg_normal_print(int, struct ustat_module*, const char*, size_t);
extern int color8_bg_bright_print(int, struct ustat_module*, const char*, size_t);
extern int xterm256_fg_print(int, struct ustat_module*, const char*, size_t);
extern int xterm256_bg_print(int, struct ustat_module*, const char*, size_t);
extern int rgb_fg_print(int, struct ustat_module*, const char*, size_t);
extern int rgb_bg_print(int, struct ustat_module*, const char*, size_t);

static struct ustat_module modules[] = {
    // pass-thru, skip str_len(m->name) bytes
    {"",       0, 0, 0, no_init, pass_print },
    {"%",      0, 0, 0, no_init, pass_print },

    {"load",   0, 0, 0, no_init, load_print },
    {"date",   0, 0, 0, no_init, date_print },
    {"uid",    0, 0, 0, no_init, uid_print  },
    {"user",   0, 0, 0, no_init, user_print },
    {"nusers", 0, 0, 0, nusers_init, nusers_print },

    // color-foo
    {"coff", 0, 0, 0, no_init, color_off},
    {"8#", 0, 0, 0, no_init, color8_fg_normal_print },
    {"8*", 0, 0, 0, no_init, color8_fg_bright_print },
    {"B#", 0, 0, 0, no_init, color8_bg_normal_print },
    {"B*", 0, 0, 0, no_init, color8_bg_bright_print },
    {"256#",  0, 0, 0, no_init, xterm256_fg_print },
    {"256*",  0, 0, 0, no_init, xterm256_bg_print },
    {"#", 0, 0, 0, no_init, rgb_fg_print },
    {"*", 0, 0, 0, no_init, rgb_bg_print },
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
        m[i]->print(1, m[i], argv[i], str_len(argv[i]));
    }

    return 0;
}

