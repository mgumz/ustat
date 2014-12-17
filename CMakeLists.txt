project(ustat)

set(ustat_src ustat.c
    ustat_helper.c
    ustat_date.c ustat_load.c ustat_pass.c
    ustat_mem.c
    ustat_user.c ustat_color.c
    ustat_procs.c ustat_cpu.c
    djb/byte_copy.c
    djb/scan_ulong.c
    djb/str_start.c djb/str_len.c djb/str_diffn.c
    djb/open_read.c
    djb/fmt_ulong.c
)

add_executable(ustat ${ustat_src})
