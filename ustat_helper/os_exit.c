#include "../djb/str.h"

#include <unistd.h>
#include <stdlib.h>


void os_exit(int code, const char* msg) {
    if (msg) {
        write(STDERR_FILENO, msg, str_len(msg));
        write(STDERR_FILENO, "\n", 1);
    }
    exit(code);
}
