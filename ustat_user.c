#include "ustat.h"
#include "djb/fmt.h"
#include "djb/str.h"
#include <unistd.h>
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

int nusers_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    char buf[6];
    int  n = 0;
    struct utmpx* u;

    for (u = getutxent(); u ; u = getutxent()) {
        if (u->ut_type == USER_PROCESS) {
            n++;
        }
    }

    n = fmt_ulong(buf, n);
    write(1, buf, n);

    return 1;
}
