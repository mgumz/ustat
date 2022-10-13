#include <sys/sysinfo.h>

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free) {

    struct sysinfo si;
    if (sysinfo(&si) == -1) {
        return 0;
    }

    *mem_total = (uint64_t)si.totalram * si.mem_unit;
    *mem_free = (uint64_t)si.freeram * si.mem_unit;

    return 1;
}

