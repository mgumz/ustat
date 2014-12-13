#ifndef _USTAT_H_
#define _USTAT_H_

#include <stddef.h>

struct ustat_module;

struct ustat_module {
    const char* name;
    char ready;
    const size_t dlen;
    const void* data;

    int (*init)(struct ustat_module*, const char* s, size_t len);
    int (*print)(int fd, struct ustat_module*, const char* s, size_t len);
};

// helper functions

extern int no_init(struct ustat_module*, const char*, size_t);
extern int print_double(int fd, double val, int prec);
extern int scan_hex(const char* s, size_t l, unsigned long* n);

#endif
