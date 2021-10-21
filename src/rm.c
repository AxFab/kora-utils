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
    OPTION('d', "dir", "remove empty directory"),
    OPTION('f', "force", "ignore nonexistent files and arguments, never prompt"),
    // OPTION('i', "prompt", "prompt before every removal"),
    OPTION('R', NULL, "alias for -r"),
    OPTION('r', "recursive", "remove directories and their contents recursively"),
    OPTION('v', "verbose", "explain what is being done"),
    END_OPTION("Copy SOURCE to DEST, or multiple SOURCE(s) to DIRECTORY.")
};

char* usages[] = {
    "[OPTION]... [FILE]...",
    NULL,
};

char* __program;

struct remove_info {
    int do_force;
    int update_only;
    int verbose;
    int is_recursive;
} _;

void rm_parse_args(void* params, unsigned char opt)
{
    switch (opt) {
        break;
    case 'f':
        _.do_force = 1;
        break;
    case 'R':
    case 'r':
        _.is_recursive = 1;
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

void do_remove_dir_entries(const char* path)
{

}

int do_remove(const char* path)
{
    struct stat st;
    int ret = stat(path, &st);
    if (S_ISLNK(st.st_mode) || S_ISREG(st.st_mode))
        return unlink(path);
    if (S_ISDIR(st.st_mode)) {
        if (_.is_recursive)
            do_remove_dir_entries(path);
        return rmdir(path);
    }

    fprintf(stderr, "Cannot remove special files\n");
    return -1;
}

int main(int argc, char** argv)
{
    int o;
    __program = argv[0];
    memset(&_, 0, sizeof(_));

    int n = arg_parse(argc, argv, (parsa_t)rm_parse_args, &_, options);
    if (n < 1) {
        fprintf(stderr, "Missing operand.\n");
        arg_usage(__program, options, usages);
        return -1;
    }

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        if (do_remove(argv[o]) != 0)
            return -1;
    }

    return 0;
}
