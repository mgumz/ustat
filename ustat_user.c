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

struct name_t {
    size_t len;
    char s[sizeof(((struct utmpx*)0)->ut_user)];
};

static int name_t_cmp(const void* va, const void* vb) {
    const struct name_t* a = (const struct name_t*)va;
    const struct name_t* b = (const struct name_t*)vb;
    const size_t l = (a->len < b->len) ? a->len : b->len;

    return str_diffn(a->s, b->s, l);
}

int nusers_init(struct ustat_module* m, const char* s, size_t l) {

    struct utmpx*  u;
    const size_t   name_len = sizeof(u->ut_user);
    const size_t   ns = sizeof(struct name_t);
    struct name_t* names;
    struct name_t* name;
    int            i, n;

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
    names = malloc(n * ns);

    setutxent();
    for (i = 0, u = getutxent(); u ; u = getutxent()) {
        if (u->ut_type == USER_PROCESS) {
            name = &names[i];
            name->len = str_len(u->ut_user);
            byte_copy(name->s, name->len, u->ut_user);
            i++;
        }
    }

    qsort(names, n, ns, name_t_cmp);

    for (nusers = 1 ,i = 1; i < n; i++) {
        if (str_diffn(names[(i - 1)].s, names[i].s, name_len) != 0) {
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
