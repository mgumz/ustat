#include "ustat.h"
#include "djb/fmt.h"
#include <unistd.h>

static uint64_t n_pages;
static uint64_t page_size;

int mem_init(struct ustat_module* m, const char* s, size_t l) {

    // equals hw.usermem (freebsd)
    long np = sysconf(_SC_PAGESIZE);
    long ps = sysconf(_SC_PHYS_PAGES);

    if (np == -1 || ps == -1) {
        return 0;
    }

    n_pages = np;
    page_size = ps;

    m->ready = 1;
    return 1;
}


int mem_avail_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    char buf[32];
    int n = fmt_ulong(buf, n_pages * page_size);
    write(fd, buf, n);

    return 1;
}

int mem_avail_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    print_ull_human(fd, n_pages * page_size);
    return 1;
}

