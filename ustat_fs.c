#include "ustat.h"
#include "djb/str.h"
#include "djb/byte.h"
#include "djb/open.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

// available modules:
// * fst - filesystem total
// * fsf - filesystem free
// * fsu - filesystem used
// * fsr - filesystem ratio (free/total
//
// since there are many filesystems, the user
// has to select it:
//
// fst/                 - total of / fileystem
// fst/other/mountpoint - total of /other/mountpoint
// 

struct ustat_fs;

struct ustat_fs {
    const char* id;
    size_t  id_l;
    int64_t n_free;
    int64_t n_total;
};

static const char error_no_fs_entries[] = "can't get a fs entry via getfsent(3)";

static int _fs_init_platform();

size_t _fs_n = 0;
struct ustat_fs* _fs = 0x0;

int fs_init(struct ustat_module *m, const char* s, size_t l) {

    if (_fs != 0x0) {
        return 1;
    }

    _fs_init_platform(&_fs, &_fs_n);


    if (_fs == 0x0) {
       write(STDERR_FILENO, error_no_fs_entries, sizeof(error_no_fs_entries));
       return 0; 
    }

    m->ready = 1;
    return 1;
}

#include <stdio.h>

#if defined(__linux__)
#include <sys/statfs.h>
#endif
#if defined(__linux__)
#include "ustat_fs_linux.c"
#else
#include "ustat_fs_other.c"
#endif




static struct ustat_fs* _fs_select(struct ustat_module* m, struct ustat_fs* fs, size_t n_fs, const char* s, size_t l) {
    struct ustat_fs* f = &fs[0];
    size_t off = str_len(m->name);
    if (off == l) {
        return f;
    }
    const char* id = &s[off];
    size_t id_l = l - off;
    size_t i;
    for (i = 0; i < n_fs; i++) {
        const size_t n = (id_l < fs[i].id_l ? id_l : fs[i].id_l);
        if (str_diffn(id, fs[i].id, n) == 0) {
           return &fs[i];
        }
    }
    return f;
}

int fs_total_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    return write_8longlong(fd, fs->n_total);
}

int fs_total_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    return write_8longlong_human(fd, 1, fs->n_total);
}


int fs_free_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    return write_8longlong(fd, fs->n_free);
}

int fs_free_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    return write_8longlong_human(fd, 1, fs->n_free);
}

int fs_used_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    return write_8longlong(fd, fs->n_total - fs->n_free);
}

int fs_used_human_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    return write_8longlong_human(fd, 1, fs->n_total - fs->n_free);
}

int fs_ratio_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    struct ustat_fs* fs = _fs_select(m, _fs, _fs_n, s, l);
    if (fs->n_total == 0) {
       return write_double(fd, 0.0, 2);
    }
    return write_double(fd, (double)fs->n_free / (double)fs->n_total, 2);
}
