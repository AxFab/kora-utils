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
    // OPTION('l', NULL, "List all filesystem"),
    // OPTION('h', NULL, "Help"),
    // OPTION('V', NULL, "Version"),

    // OPTION('a', NULL, "Mount all from /etc/fstab"),
    // OPTION('F', NULL, "Fork to mount all simultaneous"),

    // OPTION('f', "fake", "Causes everything to be done except for the actual system call"),
    // OPTION('n', NULL, "Mount without writing in /etc/mtab."),
    OPTION('r', "read-only", "Mount the filesystem read-only. A synonym is -o ro."),
    // OPTION('s', NULL, "Tolerate sloppy mount options rather than failing."),
    OPTION('v', "verbose", "Verbose mode"),
    OPTION('w', "rw", "Mount the filesystem read/write. This is the default. A synonym is -o rw."),

    OPTION_A('t', NULL, "Specify the filesystem"),
    OPTION_A('o', NULL, "Specify options"),
    // OPTION('O', NULL, ""),

    END_OPTION("Mount a filesystem.")
};

char *usages[] = {
    "[-l] [-t vfstype]",
    // "-a [-fFnrsvw] [-t vfstype] [-O optlist]",
    "[-fnrsvw] [-t vfstype] [-o option[,option]...] [device] dir",
    NULL,
};

struct {
    char *fstype;
    char *options;
    char *device;
    char *dir;
    bool ro;
    bool verbose;
    int args;
    // bool list;
} _;

void mount_parse_args(void* params, int opt, char *arg)
{
    switch (opt) {
    // case 'l':
    //     _.list = true;
    //     break;
    case 'v':
        _.verbose = true;
        break;
    case 'r':
        _.ro = true;
        break;
    case 'w':
        _.ro = false;
        break;
    case 't':
        _.fstype = strdup(arg);
        break;
    case 'o':
        _.options = strdup(arg);
        break;
    }
}

int mount_parse_names(void *params, char *name)
{
    if (_.device == NULL && _.args == 2)
        _.device = strdup(name);
    else if (_.dir == NULL)
        _.dir = strdup(name);
    return 0;
}

int main(int argc, char **argv)
{
    memset(&_, 0, sizeof(_));
    _.args = arg_parse(argc, argv, mount_parse_args, &_, options, usages);
    if (_.args == 0 || _.args > 2) {
        arg_usage(argv[0], options, usages);
        return -1;
    }
    arg_names(argc, argv, mount_parse_names, NULL, options);

    int ret = syscall(SYS_MOUNT, _.device, _.dir, _.fstype, _.options, 0);
    return ret == 0 ? 0 : -1;
}
