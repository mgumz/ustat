#include "ustat.h"
#include <unistd.h>
#include <stdlib.h>

int load_print(int fd, struct ustat_module* m, const char* s, size_t l) {

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

    return 1;
}
