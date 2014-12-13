#include "ustat.h"
#include "djb/str.h"
#include <unistd.h>

static const char* codes[] = {
    "\x1b[0m",
    "\x1b[30m",
    "\x1b[31m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[34m",
    "\x1b[35m",
    "\x1b[36m",
    "\x1b[37m",
};


int color_print(int fd, struct ustat_module* m, const char* s, size_t l) {

    int i = 0;
    switch (s[l-1]){
    case '1': i = 1; break;
    case '2': i = 2; break;
    case '3': i = 3; break;
    case '4': i = 4; break;
    case '5': i = 5; break;
    case '6': i = 6; break;
    case '7': i = 7; break;
    };

    write(fd, codes[i], str_len(codes[i]));

    return 1;
}

