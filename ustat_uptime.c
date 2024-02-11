// https://stackoverflow.com/questions/3269321/osx-programmatically-get-uptime
//

#include "ustat.h"

#include <sys/param.h>
#include <unistd.h>

static int64_t _uptime = 0;

static int _get_boottime(int64_t*);

int uptime_init(struct ustat_module* m, const char* s, size_t l) {

    _get_boottime(&_uptime);
    m->ready = 1;
    return 1;
}

int uptime_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    write_uint64(fd, _uptime);
    return 1;
}

int uptime_print_human(int fd, struct ustat_module* m, const char* s,
                       size_t l) {

    int64_t ut = _uptime;

    if (ut >= 86400) {
        write_uint64(fd, ut / 86400);
        write(fd, "d", 1);
        ut = ut % 86400;
    }
    if (ut >= 3600) {
        write_uint64(fd, ut / 3600);
        write(fd, "h", 1);
        ut = ut % 3600;
    }
    if (ut >= 60) {
        write_uint64(fd, ut / 60);
        write(fd, "m", 1);
        ut = ut % 60;
    }

    write_uint64(fd, ut);
    write(fd, "s", 1);

    return 1;
}

#if defined(BSD)
#include "ustat_uptime_bsd.c"
#else
#if defined(__linux__)
#include "ustat_uptime_linux.c"
#endif // __linux__
#endif // BSD
