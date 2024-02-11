#include "../djb/str.h"

#include <stdlib.h>

int os_getenv_or_def(const char** s, const char* env, const char* def) {

    const char* e = getenv(env);
    if (e == 0x0 || str_len(e) == 0) {
        e = def;
    }

    if (s != 0x0) {
        *s = e;
    }
    return str_len(e);
}
