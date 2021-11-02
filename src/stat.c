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
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define BUF_SZ 4096

opt_t options[] = {
    OPTION('L', "dereference", "Follow symlinks."),
    OPTION('f', "file-system", "Display file system information instead of file status."),
    OPTION('c', "format", "Use the specified FORMAT instead of the default. Output a new line after use of FORMAT"),
    OPTION(0xd3, "printf", "Like --format but interpret backslash escapes, and do not include a trailing newline."),
    OPTION('t', "terse", "Print information in terse form."),
    END_OPTION("Display file or file system status.")
};

char *usages[] = {
    "[OPTION]... FILE...",
    NULL,
};

const char *_rights[] = {
    "---", "--x", "-w-", "-wx",
    "r--", "r-x", "rw-", "rwx",
};

int do_stat(const char *path, char *buf, int len)
{
    char tmp[32];
    struct stat st;
    int ret = stat(path, &st);
    if (ret != 0) {
        fprintf(stderr, "Unable to access file %s\n", path);
        return -1;
    }

    // TODO - If symlink and dreference...

    char *format = strdup("  File: %n\n"
                          "  Size: %s\t\tBlocks: %b\t\tIO Block: %B %F\n"
                          "Device: %dh/%Dd\t\tInode: %i\t\tLinks: %h\n"
                          "Access: (%a/%A)  Uid: (%5u/%8U)  Gid: (%5g/%8G)\n"
                          "Access: %x\n"
                          "Modify: %y\n"
                          "Change: %z\n"
                          " Birth: %w\n");

    buf[0] = '\0';

    while (*format) {
        int lg = 0;
        while (format[lg] != '%' && format[lg] != '\0')
            lg++;
        if (lg != 0) {
            char t = format[lg];
            format[lg] = '\0';
            strncat(buf, format, len);
            format[lg] = t;
            format += lg;
            continue;
        }

        if (*format == '\0' || *format != '%')
            break;

        format++;
        int field_size = strtol(format, &format, 10);

        switch (*format) {
        case 'a':
            snprintf(tmp, 32, "%04o", st.st_mode & 0xfff);
            break;
        case 'A':
            if (S_ISREG(st.st_mode))
                strncpy(tmp, "-", 32);
            else if (S_ISDIR(st.st_mode))
                strncpy(tmp, "d", 32);
            else if (S_ISCHR(st.st_mode))
                strncpy(tmp, "c", 32);
            else if (S_ISBLK(st.st_mode))
                strncpy(tmp, "b", 32);
            else if (S_ISFIFO(st.st_mode))
                strncpy(tmp, "p", 32);
            else if (S_ISLNK(st.st_mode))
                strncpy(tmp, "l", 32);
            else if (S_ISSOCK(st.st_mode))
                strncpy(tmp, "s", 32);
            else
                strncpy(tmp, "?", 32);
            strncat(tmp, _rights[(st.st_mode >> 6) & 07], 32);
            strncat(tmp, _rights[(st.st_mode >> 3) & 07], 32);
            strncat(tmp, _rights[(st.st_mode >> 0) & 07], 32);
            break;
#ifndef _WIN32
        case 'b':
            snprintf(tmp, 32, "%ld", st.st_blocks);
            break;
        case 'B':
            snprintf(tmp, 32, "%ld", st.st_blksize);
            break;
#endif
        case 'd':
            snprintf(tmp, 32, "%lx", st.st_dev);
            break;
        case 'D':
            snprintf(tmp, 32, "%ld", st.st_dev);
            break;
        case 'F':
            if (S_ISREG(st.st_mode))
                strncpy(tmp, "regular file", 32);
            else if (S_ISDIR(st.st_mode))
                strncpy(tmp, "directory", 32);
            else if (S_ISCHR(st.st_mode))
                strncpy(tmp, "character device", 32);
            else if (S_ISBLK(st.st_mode))
                strncpy(tmp, "block device", 32);
            else if (S_ISFIFO(st.st_mode))
                strncpy(tmp, "pipe", 32);
            else if (S_ISLNK(st.st_mode))
                strncpy(tmp, "symlink", 32);
            else if (S_ISSOCK(st.st_mode))
                strncpy(tmp, "socket", 32);
            else
                strncpy(tmp, "-", 32);
            break;
        case 'i':
            snprintf(tmp, 32, "%ld", st.st_ino);
            break;
        case 'h':
            snprintf(tmp, 32, "%ld", st.st_nlink);
            break;
        case 'n':
            strncpy(tmp, path, 32);
            break;
        case 's':
            snprintf(tmp, 32, "%ld", st.st_size);
            break;
        case 'u':
            snprintf(tmp, 32, "%5d", st.st_uid);
            break;
        case 'U':
            snprintf(tmp, 32, "%8s", st.st_uid == 0 ? "root" : "-");
            break;
        case 'g':
            snprintf(tmp, 32, "%5d", st.st_gid);
            break;
        case 'G':
            snprintf(tmp, 32, "%8s", st.st_gid == 0 ? "root" : "-");
            break;
        case 'w':
            strncpy(tmp, "-", 32);
            break;
        case 'x':
            ctime_r(&st.st_atime, tmp);
            *strchr(tmp, '\n') = '\0';
            break;
        case 'y':
            ctime_r(&st.st_mtime, tmp);
            *strchr(tmp, '\n') = '\0';
            break;
        case 'z':
            ctime_r(&st.st_ctime, tmp);
            *strchr(tmp, '\n') = '\0';
            break;
        default:
            strncpy(tmp, "?", 32);
            break;
        }

        format++;
        (void)field_size;
        strncat(buf, tmp, len);
    }

    printf(buf);
    return 0;
}

void stat_parse_args(void *params, int opt, char *arg)
{
    switch (opt) {
    case 'L':
        break;
    case 'f' :
        break;
    case 'c' :
        break;
    case 0xd3:
        break;
    case 't' :
        break;
    }
}

int stat_main(void *params, char *arg)
{
    char buf[BUF_SZ];
    return do_stat(arg, buf, BUF_SZ);
}

int main(int argc, char **argv)
{
    int n = arg_parse(argc, argv, stat_parse_args, NULL, options, usages);
    if (n == 0) {
        fprintf(stderr, "Missing operand\n");
        return -1;
    }

    return arg_names(argc, argv, stat_main, NULL, options);
}
