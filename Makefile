SRC=ustat.c \
	ustat_color.c \
	ustat_cpu.c \
	ustat_date.c \
	ustat_fs.c \
	ustat_load.c \
	ustat_mem.c \
	ustat_pass.c \
	ustat_procs.c \
	ustat_tcp.c \
	ustat_user.c \
	ustat_helper/fmt_8ll.c \
	ustat_helper/os_exit.c \
	ustat_helper/read_double_from_fd.c \
	ustat_helper/scan_hex.c \
	ustat_helper/write_8ll.c \
	ustat_helper/write_8ll_human.c \
	ustat_helper/write_double.c \
	djb/byte_copy.c djb/byte_zero.c \
	djb/scan_ulong.c \
	djb/str_start.c djb/str_len.c djb/str_diffn.c \
	djb/open_read.c \
	djb/fmt_ulong.c

ustat: $(SRC)
	gcc -Os -o $@ -I. -Idjb $(SRC)

ustat.debug: $(SRC)
	gcc -g -o $@ -I. -Idjb $(SRC)

ustat.diet: $(SRC) ustat_fs_linux.c ustat_tcp_linux.c
	diet -Os gcc -o $@ -I. -Idjb $(SRC)
	diet -Os gcc -o $@ -I. -Idjb $(SRC)

analyze: $(SRC)
	gcc --analyze -I. -Idjb $(SRC)

docker-image:
	docker build -t ustat-dev -f docker/Dockerfile .
