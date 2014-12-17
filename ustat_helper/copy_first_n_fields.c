#include <unistd.h>

// copy the bytes from 'in_fd' to 'out_fd', stop after finding
// 'n' chars 'sep'
int first_n_fields(int out_fd, int in_fd, char sep, int n) {

    int r;
    char c;

    for ( ;; ) {

        r = read(in_fd, &c, 1);
        if (r <= 0) {
            break;
        }

        if (c == sep) {
            n--;
        }

        if (n <= 0) {
            break;
        }

        write(out_fd, &c, 1);
    }

    return 1;
}

