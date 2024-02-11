#include "djb/byte.h"
#include "djb/fmt.h"
#include "djb/open.h"
#include "djb/scan.h"

#include <unistd.h>
#include <stdint.h>

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

    unsigned long val = 0;
    scan_ulong(buf, &val);

    *uptime = val;

    return 1;
}
