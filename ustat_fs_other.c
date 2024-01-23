#include "ustat_fs.h"

#include "djb/str.h"
#include "djb/byte.h"

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>

static int _fs_init_platform(struct ustat_fs* fs[], size_t* fs_n) {

    struct statfs* buf;
    int n = getmntinfo(&buf, 0);
    if (n == 0) {
        return -1;
    }

    struct ustat_fs* f = realloc(*fs, sizeof(struct ustat_fs) * n);

    int i;
    for (i = 0; i < n; i += 1) {
        const struct statfs* sf = &buf[i];
        const int l = str_len(sf->f_mntonname);
        char* s = (char*)malloc(l);
        byte_copy(s, l + 1, sf->f_mntonname);

        f[i].id = s;
        f[i].id_l = l;
        f[i].n_free = sf->f_bfree * sf->f_bsize;
        f[i].n_total = sf->f_blocks * sf->f_bsize;
    }

    *fs = f;
    *fs_n = n;

    return 1;
}

