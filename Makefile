CC ?= gcc
CFLAGS ?=
SRC=ustat.c \
	ustat_battery.c \
	ustat_color.c \
	ustat_cpu.c \
	ustat_date.c \
	ustat_fs.c \
	ustat_load.c \
	ustat_mem.c \
	ustat_pass.c \
	ustat_procs.c \
	ustat_spec.c \
	ustat_tcp.c \
	ustat_uptime.c \
	ustat_user.c \
	ustat_helper/fmt_uint64.c \
	ustat_helper/os_exit.c \
	ustat_helper/os_getenv_def.c \
	ustat_helper/read_double_from_fd.c \
	ustat_helper/scan_hex.c \
	ustat_helper/write_uint64.c \
	ustat_helper/write_uint64_human.c \
	ustat_helper/write_double.c \
	djb/byte_copy.c djb/byte_zero.c \
	djb/scan_ulong.c \
	djb/str_start.c djb/str_len.c djb/str_diffn.c \
	djb/open_read.c \
	djb/fmt_ulong.c

ustat: $(SRC)
	$(CC) $(CFLAGS) -Os -o $@ -I. -Idjb $(SRC)

ustat.debug: $(SRC)
	$(CC) -g -o $@ -I. -Idjb $(SRC)

ustat.diet: $(SRC) ustat_fs_linux.c ustat_tcp_linux.c
	diet -Os gcc -o $@ -I. -Idjb $(SRC)
	diet -Os gcc -o $@ -I. -Idjb $(SRC)

analyze: $(SRC)
	gcc --analyze -I. -Idjb $(SRC)

docker-image:
	docker build -t ustat-dev -f docker/Dockerfile .
