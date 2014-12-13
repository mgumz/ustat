#include "ustat.h"
#include "djb/fmt.h"
#include "djb/str.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

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

