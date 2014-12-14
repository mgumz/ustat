#include "ustat.h"
#include "djb/fmt.h"
#include "djb/str.h"
#include "djb/byte.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <utmpx.h>


int user_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    struct passwd* pw = getpwuid(getuid());
    write(1, pw->pw_name, str_len(pw->pw_name));
    return 1;
}

int uid_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    char buf[6];
    uid_t uid = getuid();
    size_t n = fmt_ulong(buf, uid);
    write(fd, buf, n);

    return 1;
}



static int nusers = -1;

static int cmp_func(void* vl, const void* va, const void* vb) {
    size_t l = *(size_t*)vl;
    const char* a = (const char*)va;
    const char* b = (const char*)vb;

    return str_diffn(a, b, l);
}

int nusers_init(struct ustat_module* m, const char* s, size_t l) {

    struct utmpx*     u;
    size_t name_len = sizeof(u->ut_user);
    char*  names;
    int    i, n;

    if (nusers != -1) {
        m->ready = 1;
        return 1;
    }

    // count the number of entries. the have to be
    // sorted and made unique in order to
    // get a correct answer for the numbers of
    // active users on the system.
    //
    // TODO: there might be a problem with 2 successive
    // calls to setuxent() .. the numbers of users might
    // have changed in between. on the other hand: we
    // do not call endutxent() and thus the db might be
    // read-only for other processes

    setutxent();
    for (i = 0, u = getutxent(); u ; u = getutxent()) {
        if (u->ut_type == USER_PROCESS) {
            i++;
        }
    }

    // no need to sort anything
    if (i <= 1) {
        nusers = i;
        goto end;
    }

    n = i;
    names = malloc(n * name_len);
    setutxent();
    for (i = 0, u = getutxent(); u ; u = getutxent()) {
        if (u->ut_type == USER_PROCESS) {
            byte_copy(&names[i * name_len], name_len, u->ut_user);
            i++;
        }
    }

    qsort_r(names, n, name_len, &name_len, cmp_func);

    for (nusers = 1 ,i = 1; i < n; i++) {
        if (str_diffn(&names[(i-1)*name_len], &names[i*name_len], name_len) != 0) {
            nusers++;
        }
    }

    free(names);
end:
    endutxent();
    m->ready = 1;
    return 1;
}

int nusers_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    char buf[6];
    int  n = fmt_ulong(buf, nusers);
    write(1, buf, n);

    return 1;
}
