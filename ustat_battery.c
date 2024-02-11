#include "ustat.h"

#include "djb/fmt.h"

#include <unistd.h>

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

int bat_level_print(int fd, uint64_t val) {

    char buf[FMT_ULONG];
    int n = fmt_ulong(buf, _battery_info.level);
    write(fd, buf, n);

    return 1;
}

int bat_charging_print(int fd, uint64_t val) {

    char buf[FMT_ULONG];
    int n = fmt_ulong(buf, _battery_info.is_charging);
    write(fd, buf, n);

    return 1;
}

int bat_charging_human_print(int fd, uint64_t val) {

    const struct {
        const char* key;
        const char* def;
    } s1s2[2] = {{"USTAT_BAT_DISCHARGING", "v"}, {"USTAT_BAT_CHARGING", "^"}};

    const char* s = 0x0;
    const size_t l = os_getenv_or_def(&s, s1s2[_battery_info.is_charging].key,
                                      s1s2[_battery_info.is_charging].def);

    if (l > 0) {
        write(fd, s, l);
    }

    return 1;
}

#if defined(__APPLE__)
#include "ustat_battery_osx.c"
#else
#if defined(__linux__)
#include "ustat_battery_linux.c"
#else
static int _get_battery_info(struct ustat_battery* bi) {
    // TODO: implement
    return 0;
}
#endif
#endif
