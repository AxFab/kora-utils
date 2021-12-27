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
#include "../utils.h"
#include <stdbool.h>
#include <sys/syscall.h>

opt_t options[] = {
    OPTION('V', NULL, "Verbose output"),
    OPTION_A('t', NULL, "Specifies the type of file system to be build."),
    OPTION_A('o', NULL, "Specifies options for the file system."),
    OPTION('c', NULL, "Check for bad blocks."),

    END_OPTION("Format a disk to create a new filesystem.")
};

char *usages[] = {
    "[-V] [-t fstype] [-o option[,option]...] filesys",
    "OPTION",
    NULL,
};

struct {
    char *fstype;
    char *options;
    bool verbose;
} _;

void mkfs_parse_args(void* params, int opt, char *arg)
{
    switch (opt) {
    case 'v':
        _.verbose = true;
        break;
    case 't':
        _.fstype = strdup(arg);
        break;
    case 'o':
        _.options = strdup(arg);
        break;
    case 'c':
        break;
    }
}

int mkfs_do(void *params, char *name)
{
    int ret = syscall(SYS_MKFS, name, _.fstype, _.options, 0);
    return ret == 0 ? 0 : -1;
}

int main(int argc, char **argv)
{
    memset(&_, 0, sizeof(_));
    int n = arg_parse(argc, argv, mkfs_parse_args, &_, options, usages);
    if (n != 1) {
        fprintf(stderr, "The command take exactly one destination file.\n");
        return -1;
    }

    int ret = arg_names(argc, argv, mkfs_do, NULL, options);
    return ret == 0 ? 0 : -1;
}
