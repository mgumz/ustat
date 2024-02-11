#ifndef _USTAT_H_
#define _USTAT_H_

#include <stddef.h>
#include <stdint.h>

struct ustat_module;

struct ustat_module {
    const char* name;
    const char* descr;
    char ready;
    const size_t dlen;
    const void* data;

    int (*init)(struct ustat_module*, const char* s, size_t len);
    int (*print)(int fd, struct ustat_module*, const char* s, size_t len);
};

// helper functions

extern int fmt_uint64(char* s, uint64_t val);
extern int scan_hex(const char* s, size_t l, uint32_t* n);
extern int first_n_fields(int out_fd, int fd, char sep, int n);
extern void os_exit(int code, const char* msg);
extern int os_getenv_or_def(const char** s, const char* env, const char* def);
extern int read_double_from_fd(int fd, double* val);
extern int write_double(int fd, double val, int prec);
extern int write_uint64(int fd, uint64_t val);
extern int write_uint64_human(int fd, uint64_t min, uint64_t val);

#endif
