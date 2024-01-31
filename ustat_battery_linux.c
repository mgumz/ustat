#include "djb/open.h"
#include "djb/scan.h"
#include "djb/str.h"

#include <sys/types.h>
#include <unistd.h>

// https://www.kernel.org/doc/html/latest/power/power_supply_class.html

static int _get_battery_info(struct ustat_battery* bi) {

    const char STATUS_FILE[] = "/sys/class/power_supply/BAT0/status";
    const char CAPACITY_FILE[] = "/sys/class/power_supply/BAT0/capacity";

    // STATUS_FILE:   "Charging\n" or "Discharging\n"
    // CAPACITY_FILE: "100\n"
    const size_t MAX_BYTES = 32;

    const char CHARGING_MARKER[] = "Charging";

    // read status: charging | discharging
    // (no status file? no battery -> exit)

    int fd = open_read(STATUS_FILE);
    if (fd == -1) {
        return 0;
    }

    char buf[MAX_BYTES];
    size_t n = read(fd, buf, sizeof(buf));
    buf[n] = 0x0;
    close(fd);

    if (str_start(buf, CHARGING_MARKER)) {
        bi->is_charging = 1;
    }

    // read capacity / charging-level
    // (no capacity file? -> exit)
    fd = open_read(CAPACITY_FILE);
    if (fd == -1) {
        return 0;
    }

    n = read(fd, buf, sizeof(buf));
    buf[n] = 0x0;
    close(fd);
    unsigned long l = 0;
    scan_ulong(buf, &l);
    bi->level = l;

    return 1;
}
