#ifndef _USTAT_FS_H_
#define _USTAT_FS_H_

#include <stdlib.h>
#include <stdint.h>

struct ustat_fs;

struct ustat_fs {
    char* id;
    size_t  id_l;
    int64_t n_free;
    int64_t n_total;
};

#endif
