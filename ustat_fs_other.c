#include "djb/str.h"
#include "djb/byte.h"
#include <fstab.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/param.h>
#include <sys/mount.h>
static int _get_total_free(const char* p, int64_t* n_total, int64_t* n_free) {

    struct statfs buf;
    if (statfs(p, &buf) == 0) {
        *n_total = buf.f_blocks * buf.f_bsize;
        *n_free = buf.f_bfree * buf.f_bsize;
    }
    return 1;
}


static int _fs_init_platform(struct ustat_fs* fs[], size_t* fs_n) {
    struct fstab* e;
    for (e = getfsent(); e != NULL; e = getfsent()) {
        int64_t n_total = 0;
        int64_t n_free = 0;
        const size_t l = str_len(e->fs_file);

        _get_total_free(e->fs_file, &n_total, &n_free);

        *fs_n += 1;
        *fs = realloc(*fs, sizeof(struct ustat_fs) * *fs_n);

        (*fs)[*fs_n-1].n_total = n_total;
        (*fs)[*fs_n-1].n_free = n_free;
        (*fs)[*fs_n-1].id_l = l;
        (*fs)[*fs_n-1].id = (char*)malloc(l+1);
        byte_copy((*fs)[*fs_n - 1].id, l+1, e->fs_file);
    }
    endfsent();
    return 1;
}
