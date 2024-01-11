#include "ustat.h"
#include "djb/fmt.h"
#include <unistd.h>

struct ustat_battery;

struct ustat_battery {
    int level;
    int is_charging;
};

static struct ustat_battery _battery_info = {.level = 100, .is_charging = 0};

static int _get_battery_info(struct ustat_battery* bi);

int bat_init(struct ustat_module* m, const char* s, size_t l) {

    if (m->ready == 1) {
        return 1;
    }

    _get_battery_info(&_battery_info);

    m->ready = 1;
    return 1;
}

#if defined(__APPLE__)
#include "ustat_battery_osx.c"
#else
static int _get_battery_info(struct ustat_battery* bi) {
    // TODO: implement
    return 0;
}
#endif

int bat_level_print(int fd, uint64_t val) {

    char buf[32];
    int n = fmt_ulong(buf, _battery_info.level);
    write(fd, buf, n);

    return 1;
}

int bat_charging_print(int fd, uint64_t val) {

    char buf[32];
    int n = fmt_ulong(buf, _battery_info.is_charging);
    write(fd, buf, n);

    return 1;
}

int bat_charging_human_print(int fd, uint64_t val) {

    if (_battery_info.is_charging) {
        write(fd, "^", 1);
    } else {
        write(fd, "v", 1);
    }

    return 1;
}

