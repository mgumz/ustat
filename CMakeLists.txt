project(ustat)

set(ustat_src ustat.c
    ustat_helper.c
    ustat_date.c ustat_load.c ustat_pass.c
    ustat_user.c ustat_color.c
    djb/str_start.c djb/str_len.c
    djb/fmt_ulong.c
)

add_executable(ustat ${ustat_src})