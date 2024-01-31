#include "djb/fmt.h"
#include "ustat.h"

#include <stdint.h>
#include <sys/param.h>
#include <unistd.h>

static uint64_t _mem_total = 0;
static uint64_t _mem_free = 0;

// usefull:
//
// http://nadeausoftware.com/articles/2012/09/c_c_tip_how_get_physical_memory_size_system

static int _get_total_free(uint64_t* mem_total, uint64_t* mem_free);

int mem_init(struct ustat_module* m, const char* s, size_t l) {

    if (_mem_total > 0 && _mem_free > 0) {
        return 1;
    }

    _get_total_free(&_mem_total, &_mem_free);

    m->ready = 1;
    return 1;
}

#if defined(BSD)
#if defined(__APPLE__)
#include "ustat_mem_osx.c"
#else // *BSD
#include "ustat_mem_bsd.c"
#endif // *BSD

#else // LINUX (or other)
#include "ustat_mem_linux.c"
#endif

static int mem_print(int fd, uint64_t val) {
    char buf[FMT_ULONG];
    int n = fmt_uint64(buf, val);
    write(fd, buf, n);
    return 1;
}

int mem_total_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return mem_print(fd, _mem_total);
}

int mem_free_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return mem_print(fd, _mem_free);
}

int mem_total_human_print(int fd, struct ustat_module* m, const char* s,
                          size_t l) {
    return write_uint64_human(fd, 1, _mem_total);
}

int mem_free_human_print(int fd, struct ustat_module* m, const char* s,
                         size_t l) {
    return write_uint64_human(fd, 1, _mem_free);
}

int mem_ratio_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return write_double(fd, (double)_mem_free / (double)_mem_total, 2);
}
