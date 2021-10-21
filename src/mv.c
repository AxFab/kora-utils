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
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include "utils.h"

int do_copy(const char* dst, const char* src);

opt_t options[] = {
    OPTION('b', "backup", "make a backup of each existing destination file"),
    OPTION('f', "force", " if an existing destination file cannot be opened, remove it and try again(this option is ignored when the -n option is also used)"),
    OPTION('n', "no-clobber ", "do not overwrite an existing file"),
    // OPTION('u', "update", "copy only when the SOURCE file is newer than the destination file or when the destination file is missing"),
    OPTION('v', "verbose", "explain what is being done"),
    END_OPTION("Copy SOURCE to DEST, or multiple SOURCE(s) to DIRECTORY.")
};

char* usages[] = {
    // "[OPTION]... [-T] SOURCE DEST",
    "[OPTION]... SOURCE DEST",
    "[OPTION]... SOURCE... DIRECTORY",
    // "[OPTION]... -t DIRECTORY SOURCE...",
    NULL,
};

char* __program;

struct move_info {
    int follow_symlink;
    int do_backup;
    int do_force;
    int no_clobber;
    int update_only;
    int verbose;
    int dst_is_directory;
} _;

void mv_parse_args(void* params, unsigned char opt)
{
    switch (opt) {
    case 'b':
        _.do_backup = 1;
        break;
    case 'f':
        _.do_force = 1;
        break;
    case 'n':
        _.no_clobber = 1;
        break;
    case 'u':
        _.update_only = 1;
        break;
    case 'v':
        _.verbose = 1;
        break;
    case OPT_HELP: // --help
        arg_usage(__program, options, usages);
        exit(0);
    case OPT_VERS: // --version
        arg_version(__program);
        exit(0);
    default:
        fprintf(stderr, "Option -%c non recognized.\n" HELP, opt, __program);
        exit(1);
    }
}


int do_move(const char* dst, const char* src)
{
    return -1;
}

int main(int argc, char** argv)
{
    int o;
    __program = argv[0];
    memset(&_, 0, sizeof(_));

    int n = arg_parse(argc, argv, (parsa_t)mv_parse_args, &_, options);
    if (n < 2) {
        fprintf(stderr, "Missing operand.\n");
        arg_usage(__program, options, usages);
        return -1;
    }
    else if (n > 2) {
        _.dst_is_directory = 1;
    }

    char* destination = NULL;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        destination = argv[o];
    }

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        if (destination == argv[o])
            break;
        if (do_move(destination, argv[o]) != 0)
            return -1;
    }

    return 0;
}
