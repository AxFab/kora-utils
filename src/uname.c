/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2019  <Fabien Bavent>
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

void readfile(const char* path, char *buf, size_t len)
{
    int fd = open(path, O_RDONLY);
    read(fd, buf, len);
    close(fd);

    char *p = strchr(buf, '\n');
    if (p)
        *p = '\0';
}

#define SHOW_NAME 1
#define SHOW_HOST 2
#define SHOW_RELEASE 4
#define SHOW_VERSION 8
#define SHOW_ARCH 16
#define SHOW_PROC 32
#define SHOW_VENDOR 64
#define SHOW_OS 128

int main(int argc, char **argv)
{
    int show = 0;
    int o, n = 0;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] != '-') {
            n++;
            continue;
        }

        unsigned char *opt = (unsigned char *)&argv[o][1];
        if (*opt == '-')
            opt = arg_long(&argv[o][2], options);
        for (; *opt; ++opt) {
            switch (*opt) {
            case 'a':
                show = 0xff;
                break;
            case 's':
                show |= SHOW_NAME;
                break;
            case 'n':
                show |= SHOW_HOST;
                break;
            case 'r':
                show |= SHOW_RELEASE;
                break;
            case 'v':
                show |= SHOW_VERSION;
                break;
            case 'm':
                show |= SHOW_ARCH;
                break;
            case 'p':
                show |= SHOW_PROC;
                break;
            case 'i':
                show |= SHOW_VENDOR;
                break;
            case 'o':
                show |= SHOW_OS;
                break;
            case OPT_HELP: // --help
                arg_usage(argv[0], options, usages);
                return 0;
            case OPT_VERS: // --version
                arg_version(argv[0]);
                return 0;
            }
        }
    }

    char buf[64];

    if (show & SHOW_NAME || show == 0) {
        readfile("/proc/sys/kernel/ostype", buf, 64);
        printf("%s ", buf);
    }

    if (show & SHOW_HOST) {
        gethostname(buf, 64);
        printf("%s ", buf);
    }

    if (show & SHOW_RELEASE) {
        readfile("/proc/sys/kernel/osrelease", buf, 64);
        printf("%s ", buf);
    }

    if (show & SHOW_VERSION) {
        readfile("/proc/sys/kernel/version", buf, 64);
        printf("%s ", buf);
    }

    if (show & SHOW_ARCH) {
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

    if (show & SHOW_PROC) {
        // printf("%s ", "i486");
    }

    if (show & SHOW_VENDOR) {
        // printf("%s ", "pc");
    }

    if (show & SHOW_OS) {
#if defined __kora__
        printf("%s ", "KoraOS");
#elif defined __linux__
        printf("%s ", "GNU/Linux");
#else
        printf("%s ", "-");
#endif
    }

    printf("\n");
    return 0;
}

