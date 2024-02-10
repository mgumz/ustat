PROJECT(ustat C)
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

SET(ustat_src
    ustat.c
    ustat_battery.c
    ustat_color.c
    ustat_cpu.c
    ustat_date.c
    ustat_fs.c
    ustat_load.c
    ustat_mem.c
    ustat_tcp.c
    ustat_pass.c
    ustat_procs.c
    ustat_spec.c
    ustat_uptime.c
    ustat_user.c
    ustat_helper/fmt_uint64.c
    ustat_helper/os_exit.c
    ustat_helper/read_double_from_fd.c
    ustat_helper/scan_hex.c
    ustat_helper/write_uint64.c
    ustat_helper/write_uint64_human.c
    ustat_helper/write_double.c
    djb/byte_copy.c djb/byte_zero.c
    djb/scan_ulong.c
    djb/str_start.c djb/str_len.c djb/str_diffn.c
    djb/open_read.c
    djb/fmt_ulong.c
)

# check for netlink-support
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    INCLUDE(CheckTypeSize)
    SET(CMAKE_EXTRA_INCLUDE_FILES netinet/in.h linux/socket.h linux/netlink.h linux/rtnetlink.h linux/inet_diag.h)
    CHECK_TYPE_SIZE("struct inet_diag_req_v23" USTAT_NETLINK)
    IF(USTAT_NETLINK)
        ADD_DEFINITIONS(-DUSTAT_NETLINK=1)
    ELSE(USTAT_NETLINK)
        MESSAGE(WARNING "no compile-time support for netlink found: 'tcp'-module uses only the /proc/net/tcp*-parser.")
    ENDIF(USTAT_NETLINK)
    SET(CMAKE_EXTRA_INCLUDE_FILES)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

ADD_EXECUTABLE(ustat ${ustat_src})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    TARGET_LINK_LIBRARIES(ustat PUBLIC "-framework IOKit -framework CoreFoundation")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

