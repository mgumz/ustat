#include "ustat.h"
#include "djb/str.h"
#include "djb/fmt.h"
#include "djb/scan.h"

#include <unistd.h>

// http://en.wikipedia.org/wiki/ANSI_escape_code
// http://ascii-table.com/ansi-escape-sequences.php


int color_off(int fd, struct ustat_module* m, const char* s, size_t l) {
    write(fd, "\x1b[0m", 4);
    return 1;
}


//
// 8color functions

static int color8_print(int fd, char c, int offset) {

    static const char* color8[] = {
        "\x1b[30m", "\x1b[30;1m", "\x1b[40m", "\x1b[40;1m",
        "\x1b[31m", "\x1b[31;1m", "\x1b[41m", "\x1b[41;1m",
        "\x1b[32m", "\x1b[32;1m", "\x1b[42m", "\x1b[42;1m",
        "\x1b[33m", "\x1b[33;1m", "\x1b[43m", "\x1b[43;1m",
        "\x1b[34m", "\x1b[34;1m", "\x1b[44m", "\x1b[44;1m",
        "\x1b[35m", "\x1b[35;1m", "\x1b[45m", "\x1b[45;1m",
        "\x1b[36m", "\x1b[36;1m", "\x1b[46m", "\x1b[46;1m",
        "\x1b[37m", "\x1b[37;1m", "\x1b[47m", "\x1b[47;1m",
    };

    int i = c - '0';
    if (i < 0 || i > 8) {
        return 0;
    }

    i = (i * 4) + offset;
    write(fd, color8[i], str_len(color8[i]));

    return 1;
}

int color8_fg_normal_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return color8_print(fd, s[l-1], 0);
}

int color8_fg_bright_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return color8_print(fd, s[l-1], 1);
}

int color8_bg_normal_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return color8_print(fd, s[l-1], 2);
}

int color8_bg_bright_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return color8_print(fd, s[l-1], 3);
}


//
// 256color functions
//
// http://pln.jonas.me/xterm-colors


static const char XTERM256_FG[] = "\x1b[38;5;";
static const char XTERM256_BG[] = "\x1b[48;5;";

static int xterm256_print(int fd, struct ustat_module* m, const char* s, size_t l, 
    const char* mode, size_t ml) {

    int off = str_len(m->name);
    unsigned long color;
    int n = scan_ulong(&s[off], &color);

    if (n == 0 || color >= 256) {
        return 0;
    }

    write(fd, mode, ml);
    write(fd, &s[off], n);
    write(fd, "m", 1);

    return 1;
}

int xterm256_fg_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return xterm256_print(fd, m, s, l, XTERM256_FG, sizeof(XTERM256_FG)-1);
}

int xterm256_bg_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return xterm256_print(fd, m, s, l, XTERM256_BG, sizeof(XTERM256_BG)-1);
}

//
// rgb-color

static const char XTERM_RGB_FG[] = "\x1b[38;2;";
static const char XTERM_RGB_BG[] = "\x1b[48;2;";

static int rgb_print(int fd, struct ustat_module* m, const char* s, size_t l,
    const char* mode, size_t ml) {

    unsigned long r = 0, g = 0, b = 0;
    int i;
    size_t off = str_len(m->name);
    size_t n = l - off;
    char buf[3];

    switch (n) {
    case 3:
        i = scan_hex(&s[off], 1, &r);
        i += scan_hex(&s[off+1], 1, &g);
        i += scan_hex(&s[off+2], 1, &b);
        r = (r << 4) + r;
        g = (g << 4) + g;
        b = (b << 4) + b;
        break;
    case 6:
        i =  scan_hex(&s[off], 2, &r);
        i += scan_hex(&s[off+2], 2, &g);
        i += scan_hex(&s[off+4], 2, &b);
        break;
    default: return 0;
    }

    if (i != n) {
        return 0;
    }

    write(fd, mode, ml);

    n = fmt_ulong(buf, r);
    write(fd, buf, n);
    write(fd, ";", 1);

    n = fmt_ulong(buf, r);
    write(fd, buf, n);
    write(fd, ";", 1);

    n = fmt_ulong(buf, r);
    write(fd, buf, n);
    write(fd, ";", 1);

    write(fd, "m", 1);

    return 1;
}

int rgb_fg_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return rgb_print(fd, m, s, l, XTERM_RGB_FG, sizeof(XTERM_RGB_FG)-1);
}

int rgb_bg_print(int fd, struct ustat_module* m, const char* s, size_t l) {
    return rgb_print(fd, m, s, l, XTERM_RGB_BG, sizeof(XTERM_RGB_BG)-1);
}

