#include <time.h>
#include <sys/time.h>
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
