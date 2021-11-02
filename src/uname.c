/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#include "utils.h"
#include <unistd.h>
#include <fcntl.h>

opt_t options[] = {
    OPTION('a', "all", "Print all information"),
    OPTION('s', NULL, "Print the kernel name (default)"),
    OPTION('n', NULL, "Print the machine hostname"),
    OPTION('r', NULL, "Print the kernel release"),
    OPTION('v', NULL, "Print the kernel version"),
    OPTION('m', NULL, "Print the architecture name"),
    OPTION('p', NULL, "Print the processor type"),
    OPTION('i', NULL, "Print hardware platform"),
    OPTION('o', NULL, "Print the OS name"),
    END_OPTION("Display system information.")
};

char *usages[] = {
    "OPTION",
    NULL,
};

#define SHOW_NAME 1
#define SHOW_HOST 2
#define SHOW_RELEASE 4
#define SHOW_VERSION 8
#define SHOW_ARCH 16
#define SHOW_PROC 32
#define SHOW_VENDOR 64
#define SHOW_OS 128


#if defined __kora__
#  define UNAME_PATH_NAME  "/proc/sys/kernel/ostype"
#  define UNAME_PATH_RELEASE  "/proc/sys/kernel/osrelease"
#  define UNAME_PATH_VERSION  "/proc/sys/kernel/version"
#  define UNAME_OS  "KoraOS"
#elif defined __linux__
#  define UNAME_PATH_NAME  "/proc/sys/kernel/ostype"
#  define UNAME_PATH_RELEASE  "/proc/sys/kernel/osrelease"
#  define UNAME_PATH_VERSION  "/proc/sys/kernel/version"
#  define UNAME_OS  "GNU/Linux"
#else
#  define UNAME_PATH_NAME  NULL
#  define UNAME_PATH_RELEASE  NULL
#  define UNAME_PATH_VERSION  NULL
#  define UNAME_OS  "-"
#endif

void readfile(const char *path, char *buf, size_t len)
{
    if (path == NULL) {
        buf[0] = '\0';
        return;
    }

    int fd = open(path, O_RDONLY);
    read(fd, buf, len);
    close(fd);

    char *p = strchr(buf, '\n');
    if (p)
        *p = '\0';
}


struct {
    int show;
} _;

void uname_parse(void *cfg, int opt, char *arg)
{
    switch (opt) {
    case 'a':
        _.show = 0xff;
        break;
    case 's':
        _.show |= SHOW_NAME;
        break;
    case 'n':
        _.show |= SHOW_HOST;
        break;
    case 'r':
        _.show |= SHOW_RELEASE;
        break;
    case 'v':
        _.show |= SHOW_VERSION;
        break;
    case 'm':
        _.show |= SHOW_ARCH;
        break;
    case 'p':
        _.show |= SHOW_PROC;
        break;
    case 'i':
        _.show |= SHOW_VENDOR;
        break;
    case 'o':
        _.show |= SHOW_OS;
        break;
    }
}

int main(int argc, char **argv)
{
    _.show = 0;
    arg_parse(argc, argv, uname_parse, NULL, options, usages);

    char buf[64];

    if (_.show & SHOW_NAME || _.show == 0) {
        readfile(UNAME_PATH_NAME, buf, 64);
        printf("%s ", buf);
    }

    if (_.show & SHOW_HOST) {
        gethostname(buf, 64);
        printf("%s ", buf);
    }

    if (_.show & SHOW_RELEASE) {
        readfile(UNAME_PATH_RELEASE, buf, 64);
        printf("%s ", buf);
    }

    if (_.show & SHOW_VERSION) {
        readfile(UNAME_PATH_VERSION, buf, 64);
        printf("%s ", buf);
    }

    if (_.show & SHOW_ARCH) {
#ifdef __ARCH
        printf("%s ", __ARCH);
#elif defined __amd64 || defined __x86_64 || defined _M_IA64
        printf("%s ", "x86_64");
#elif defined __i386 || defined _M_IX86
        printf("%s ", "i386");
#elif defined __arm__ || defined _M_ARM
        printf("%s ", "aarch64");
#else
        printf("%s ", "-");
#endif
    }

    if (_.show & SHOW_PROC) {
        // printf("%s ", "i486");
    }

    if (_.show & SHOW_VENDOR) {
        // printf("%s ", "pc");
    }

    if (_.show & SHOW_OS) {
        printf("%s ", UNAME_OS);
    }

    printf("\n");
    return 0;
}
