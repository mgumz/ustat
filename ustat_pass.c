#include "ustat.h"
#include "djb/str.h"
#include <unistd.h>
#include <stdint.h>

int pass_print(int fd, struct ustat_module* m, const char* s, size_t len) {
    size_t off = str_len(m->name);
    write(1, &s[off], len-off);
    return 1;
}
