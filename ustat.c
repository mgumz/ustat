// ustat - mikro-stat, tool to collect system-information
// and print it to stdout.

#include "ustat.h"
#include "djb/str.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//
// modules
extern int pass_print(int fd, struct ustat_module*, const char* s, size_t len);
extern int load_print(int fd, struct ustat_module*, const char* s, size_t len);
extern int date_print(int fd, struct ustat_module*, const char* s, size_t len);
extern int user_print(int fd, struct ustat_module*, const char* s, size_t len);
extern int uid_print(int fd, struct ustat_module*, const char* s, size_t len);


//
static struct ustat_module modules[] = {
    {"pass", 0, 0, 0, no_init, pass_print },
    {"load", 0, 0, 0, no_init, load_print },
    {"date", 0, 0, 0, no_init, date_print },
    {"uid",  0, 0, 0, no_init, uid_print  },
    {"user", 0, 0, 0, no_init, user_print },
};
static const size_t nmodules = sizeof(modules)/sizeof(struct ustat_module);

// usage: ustat "load.1" ":" "load.2" ":" "load.3"
int main(int argc, char* argv[]) {

    struct ustat_module** m = alloca(argc * sizeof(struct ustat_module*));

    // first, scan the given args and init() any module which is
    // not yet ready.
    int i, j;
    for (i = 1; i < argc; i++) {
        for (j = 0; j < nmodules; j++) {
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

