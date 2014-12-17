#include <stddef.h>
#include <ctype.h>

int scan_hex(const char* s, size_t l, unsigned long* n) {

    char c;
    size_t i;
    for (i = 0 ;i < l; i++) {

        // '0 - '9'
        c = s[i] - '0';
        if (c < 0) {
            return i;
        } else if (c <= 9) {
            goto add;
        }

        // 'a' - 'f'
        c = tolower(s[i]) - 'a';
        if (c < 0 || c > 5) {
            return i;
        }
        c += 10;
add:
        if (n) {
            *n = (*n << 4) + c;
        }
    }
    return i;
}

