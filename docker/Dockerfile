FROM debian:buster AS base-env

RUN  apt update && \
     apt dist-upgrade -y
RUN  apt install -y --no-install-recommends \
        cmake \
        dietlibc-dev \
        gcc \
        gdb \
        libc-dev \
        libowfat-dietlibc-dev \
        make

FROM base-env AS build-env

COPY . /src/

RUN  cd /src/ && make ustat.diet

