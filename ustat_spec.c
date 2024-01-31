#include "ustat.h"

#include <unistd.h>

int nl_print(int fd, struct ustat_module* m, const char* s, size_t len) {
    write(1, "\n", 1);
    return 1;
}
