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

#if defined(__APPLE__)
#include <time.h>
#include <sys/sysctl.h>
#include <sys/types.h>

static int _get_boottime(int64_t* uptime) {

    struct timeval bt;
    size_t s = sizeof(bt);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};

    if (sysctl(mib, 2, &bt, &s, 0, 0) == -1) {
        return 0;
    }

    struct timeval now;
    gettimeofday(&now, NULL);

    *uptime = now.tv_sec - bt.tv_sec;

    return 1;
}

#else
#if defined(__linux__)
#include "djb/byte.h"
#include "djb/fmt.h"
#include "djb/open.h"
#include "djb/scan.h"

static int _get_boottime(int64_t* uptime) {

    const char UPTIME_FILE[] = "/proc/uptime";
    char buf[FMT_ULONG];
    int fd = open_read(UPTIME_FILE);
    if (fd == -1) {
        return 0;
    }

    size_t n = read(fd, buf, sizeof(buf));
    buf[n] = 0x0;
    close(fd);

    unsigned long l = 0;
    scan_ulong(buf, &l);

    *uptime = l;

    return 1;
}

#endif

#endif
