#include "ustat.h"
#include <unistd.h>

int pass_print(int fd, struct ustat_module* m, const char* s, size_t len) {
    write(1, s, len);
    return 1;
}
