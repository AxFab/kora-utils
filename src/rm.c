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
#include <sys/stat.h>
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

struct remove_info {
    int do_force;
    int update_only;
    int verbose;
    int is_recursive;
} _;

void rm_parse_args(void* params, int opt, char *arg)
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
    }
}

void do_remove_dir_entries(const char* path)
{

}

int do_remove(void *cfg, char* path)
{
    struct stat st;
    int ret = stat(path, &st);
    if (ret != 0) {
        if (_.do_force)
            return 0;
        fprintf(stderr, "Unable to find files %s\n", path);
        return -1;
    }
    if (S_ISLNK(st.st_mode) || S_ISREG(st.st_mode))
        return unlink(path);
    if (S_ISDIR(st.st_mode)) {
        if (_.is_recursive)
            do_remove_dir_entries(path);
        return rmdir(path);
    }

    fprintf(stderr, "Cannot remove special files %s\n", path);
    return -1;
}

int main(int argc, char** argv)
{
    memset(&_, 0, sizeof(_));
    int n = arg_parse(argc, argv, rm_parse_args, &_, options, usages);
    if (n == 0) {
        fprintf(stderr, "Missing operand.\n");
        return -1;
    }

    return arg_names(argc, argv, do_remove, &_, options);
}
