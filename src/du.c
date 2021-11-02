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
#include <sys/stat.h>
#include <unistd.h>

opt_t options[] = {
    OPTION('a', "all", "write count for all file, not just directory."),
    OPTION(0xd3, "apparent-size", "print apparent size rather than disk usage."),
    OPTION('c', "total", "Show the grand total"),
    OPTION('D', "dereference-args", "Dereference only symlinks that are present on the command line"),
    OPTION('h', "human-readable", "Print size into a human readable format"),
    OPTION('L', "dereference", "Dereference all symbolic links"),
    OPTION('l', "count-link", "Count size many time if linked"),
    OPTION('P', "no-dereference", "Do not follow any symbolic link (default)"),
    OPTION('S', "seperate-dirs", "For directory, do not include size of sub-directories"),
    OPTION('H', "si", "like -h but use powers of 1000 unstead of 1024"),
    OPTION('s', "summarize", "Display only total for each argument"),
    OPTION('x', "one-file-system", "Do not cross file systems boundries"),
    END_OPTION("Summarize disk usage of the set of FILEs, recursively for directories.")
};
char* usages[] = {
    "[OPTION]... [FILE]...",
    NULL,
};

struct dopt {
    int follow_symlink;
    int follow_first_symlink;
    int apparent_size;
    int max_depth;
    unsigned exponent;
} _;

void du_parse_args(void* params, int opt, char *arg)
{
    switch(opt) {
    }
}

void do_print_size(const char* path, size_t size)
{
    char szbuf[32];
    if (0)
        snprintf(szbuf, 32, "%-7llu", size / 1024LLU);
    else {

        unsigned prec = 0, exp = 0;
        while (size > _.exponent) {
            prec = (size % _.exponent) * 10 / _.exponent;
            size = size / _.exponent;
            exp++;
        }

        if (exp == 0)
            snprintf(szbuf, 32, "%4u by", (int)size);
        else
            snprintf(szbuf, 32, "%4u.%01u%c", (int)size, prec, " KMGTPEYZ"[exp]);
    }

    fprintf(stdout, "%7s %s\n", szbuf, path);
}

size_t do_count_dir(const char* path, int depth)
{
    return 0;
}


size_t do_count(const char *path, int depth)
{
    struct stat st;
    if (_.follow_symlink || (depth == 0 && _.follow_first_symlink))
        lstat(path, &st);
    else
        stat(path, &st);

    if (S_ISDIR(st.st_mode))
        return do_count_dir(path, depth);
    
#ifdef _WIN32
    size_t len = st.st_size;
#else
    size_t len = _.apparent_size ? st.st_size : st.st_blocks * st.st_blksize;
#endif
    if (depth <= _.max_depth)
        do_print_size(path, len);

    return len;
}

int do_disk_usage(void *cfg, char *path)
{
    do_count(path, 0);
    return 0;
}

int main(int argc, char** argv)
{
    arg_parse(argc, argv, du_parse_args, NULL, options, usages);
    arg_names(argc, argv, do_disk_usage, NULL, options);
	return 0;
}