#include "ustat.h"
#include "djb/open.h"
#include <unistd.h>
#include <stdlib.h>

int load_print(int fd, struct ustat_module* m, const char* s, size_t l) {

#if 0
    double load[3];
    int n = getloadavg(load, 3);
    char which = s[l-1];

    switch(which) {
    case '1':
        print_double(fd, load[0], 2);
        break;
    case '2':
        print_double(fd, load[1], 2);
        break;
    case '3':
        print_double(fd, load[2], 2);
        break;
    }
#else
    // linux
    int avg_fd = open_read("/proc/loadavg");
    if (avg_fd == -1) {
        return 0;
    }
    first_n_fields(fd, avg_fd, ' ', 3);
    return 1;
#endif
}

int print_load_avg(int out_fd) {
    int fd = open_read("/proc/loadavg");
    if (fd == -1) {
        return 0;
    }
    return first_n_fields(out_fd, fd, ' ', 3);
}


