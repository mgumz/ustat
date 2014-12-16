#include "ustat.h"
#include "djb/fmt.h"
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>

#ifdef __dietlibc__
#ifndef _SC_AVPHYS_PAGES
#define _SC_AVPHYS_PAGES (_SC_PHYS_PAGES + 1)
#endif
#endif

// usefull:
//
// http://nadeausoftware.com/articles/2012/09/c_c_tip_how_get_physical_memory_size_system


static uint64_t _total_ram = 0;
static uint64_t _free_ram = 0;

#include <stdio.h>
#include <sys/sysinfo.h>

int mem_init(struct ustat_module* m, const char* s, size_t l) {

    if (_total_ram > 0 && _free_ram > 0) {
        return 1;
    }

#if defined(BSD)
    // equals hw.usermem (freebsd)
    long ps = sysconf(_SC_PAGESIZE);
    long pp = sysconf(_SC_PHYS_PAGES);
    long ap = sysconf(_SC_AVPHYS_PAGES);

    if (ps == -1 || pp == -1 || ap == -1) {
        return 0;
    }

    _total_ram = (uint64_t)ps * (uint64_t)pp;
    _free_ram = (uint64_t)ps * (uint64_t)ap;
#else
    struct sysinfo si;

    if (sysinfo(&si) == -1) {
        return 0;
    }

    _total_ram = (uint64_t)si.totalram * si.mem_unit;
    _free_ram = (uint64_t)si.freeram * si.mem_unit;
#endif

    m->ready = 1;
    return 1;
}

static int mem_print(int fd, uint64_t val) {
    char buf[32];
    int n = fmt_8longlong(buf, val);
    write(fd, buf, n);
    return 1;
}

int mem_total_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return mem_print(fd, _total_ram);
}

int mem_free_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return mem_print(fd, _free_ram);
}

int mem_total_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print_8longlong_human(fd, 1, _total_ram);
}

int mem_free_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print_8longlong_human(fd, 1, _free_ram);
}

int mem_ratio_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return print_double(fd, (double)_free_ram / (double)_total_ram, 2);
}

