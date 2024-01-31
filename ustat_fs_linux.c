#include "ustat_fs.h"

#include "djb/byte.h"
#include "djb/open.h"
#include "djb/str.h"

#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>

static int _add_mount_entry(struct ustat_fs* fs[], size_t* fs_n,
                            const char* mnt, size_t l);
static int _get_total_free(const char* p, int64_t* n_total, int64_t* n_free);

static int _fs_init_platform(struct ustat_fs* fs[], size_t* fs_n) {

    // we parse /proc/self/mountinfo
    // https://www.kernel.org/doc/Documentation/filesystems/proc.txt
    //
    // https://stackoverflow.com/questions/5713451/is-it-safe-to-parse-a-proc-file
    //
    // alternatives not picked
    // * getmntent: requires <stdio> and <stdio> is big
    // * findmnt -l -n -o TARGET: exec other binary, dependency
    // * df -o TARGET: exec other binary, dependency
    //
    // quoting the linux-kernel documentation:
    //
    // 3.5 /proc/<pid>/mountinfo - Information about mounts
    // --------------------------------------------------------
    //
    // This file contains lines of the form:
    //
    // 36 35 98:0 /mnt1 /mnt2 rw,noatime master:1 - ext3 /dev/root
    // rw,errors=continue (1)(2)(3)   (4)   (5)      (6)      (7)   (8) (9) (10)
    // (11)
    //
    // (1) mount ID:  unique identifier of the mount (may be reused after
    // umount) (2) parent ID:  ID of parent (or of self for the top of the mount
    // tree) (3) major:minor:  value of st_dev for files on filesystem (4) root:
    // root of the mount within the filesystem (5) mount point:  mount point
    // relative to the process's root (6) mount options:  per mount options (7)
    // optional fields:  zero or more fields of the form "tag[:value]" (8)
    // separator:  marks the end of the optional fields (9) filesystem type:
    // name of filesystem of the form "type[.subtype]" (10) mount source:
    // filesystem specific information or "none" (11) super options:  per super
    // block options

    const char MOUNTINFO[] = "/proc/self/mountinfo";
    const int FIELD_MOUNT_ROOT = 4;
    const int FIELD_MOUNT_POINT = 5;

    int fd = open_read(MOUNTINFO);
    if (fd == -1) {
        return 0;
    }

    size_t i;
    size_t n;

    // phase-1: consume the file to find out about its size
    for (i = 1024, n = 0;; i *= 2) {
        char b[i];
        size_t r = read(fd, b, sizeof(b));
        if (r == 0) {
            break;
        }
        if (r == -1) {
            return 0;
        }
        n += r;
    }

    // phase-2: consume the whole file at once, assume it
    // stayed "almost" the same.
    lseek(fd, 0, SEEK_SET);
    char buf[2 * n]; // to be safe, use twice the buffer size found in phase-1
    n = read(fd, buf, sizeof(buf));
    close(fd);

    // phase-3: analyze the file
    const char* mnt = 0x0;
    int field = 0;

    for (i = 0; i < n; i++) {

        char b = buf[i];
        if (b == ' ' || b == '\t') {
            field += 1;
            if (field == FIELD_MOUNT_ROOT) {
                continue;
            }
            buf[i] = 0x0;

            if (field == FIELD_MOUNT_POINT) {
                _add_mount_entry(fs, fs_n, mnt, &buf[i] - mnt);
            }
        }

        if (b == '\n') {
            goto next_line;
        }

        if (field == FIELD_MOUNT_ROOT) {
            if (mnt == 0x0) {
                mnt = &buf[i];
            }
        }

        continue;

    next_line:
        field = 0;
        mnt = 0x0;
    }

    return 1;
}

static int _add_mount_entry(struct ustat_fs* fs[], size_t* fs_n,
                            const char* mnt, size_t l) {

    // ignore some
    if (str_start(mnt, "/proc") == 1) {
        return 1;
    }
    if (str_start(mnt, "/sys") == 1) {
        return 1;
    }
    if (str_start(mnt, "/dev") == 1) {
        return 1;
    }

    size_t n = *fs_n + 1;
    int64_t n_total = 0;
    int64_t n_free = 0;

    _get_total_free(mnt, &n_total, &n_free);
    if (n_total != 0) {
        struct ustat_fs* f = realloc(*fs, sizeof(struct ustat_fs) * n);

        f[n - 1].n_total = n_total;
        f[n - 1].n_free = n_free;
        f[n - 1].id_l = l;
        f[n - 1].id = (char*)malloc(l + 1);
        byte_copy(f[n - 1].id, l + 1, mnt);
        *fs = f;
        *fs_n = n;
    }

    return 1;
}

static int _get_total_free(const char* p, int64_t* n_total, int64_t* n_free) {

    struct statvfs buf;
    if (statvfs(p, &buf) == 0) {
        *n_total = buf.f_blocks * buf.f_frsize;
        *n_free = buf.f_bfree * buf.f_frsize;
    }

    return 1;
}
