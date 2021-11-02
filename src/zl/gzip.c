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
#include <zlib.h>
#include <fcntl.h>
#include <stdbool.h>

#ifndef O_BINARY
# define O_BINARY 0
#endif

#define BUF_SZ 512

opt_t options[] = {
    OPTION('c', "stdout", "Write output on standard output ; keep original files unchanged (default)"),
    OPTION('d', "decompress", "Decompress input files"),
    // OPTION('f', "force", "..."),
    // OPTION('l', "list", "Print info for each compressed file"),
    // OPTION('n', "no-name", "..."),
    // OPTION('N', "name", "..."),
    OPTION_A('o', "output", "Write output on a file"),
    OPTION_A('s', "suffix", "..."),
    // OPTION('q', "quiet", "Suppress all warnings"),
    // OPTION('r', "recursive", "..."),
    // OPTION('v', "verbose", "..."),
    // OPTION('1', "fast", "..."),
    // OPTION('9', "best", "..."),
    END_OPTION("Into standard output.")
};

char *usages[] = {
    "OPTION... FILE...",
    NULL,
};

struct {
    int level;
    int verbose;
    bool compress;
    const char *suffix;
    int fout;

} _;

void gzip_param(void *cfg, int opt, char *arg)
{
    switch (opt) {
    case 'c':
        if (_.fout != 1)
            close(_.fout);
        _.fout = 1;
        break ;
    case 'd' :
        _.compress = false;
        break;
    case 'f' :
        break ;
    case 'l' :
        break ;
    case 'L' :
        fprintf(stderr, "This utility link to zlib library:\n");
        fprintf(stderr, "Copyright (C) 2017 Free Software Foundation, Inc.\n");
        fprintf(stderr, "Copyright (C) 1993 Jean-loup Gailly.\n");
        fprintf(stderr, "This is free software.  You may redistribute copies of it under the terms of\n");
        fprintf(stderr, "the GNU General Public License <https://www.gnu.org/licenses/gpl.html>.\n");
        fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
        exit(0);
    case 'n' :
        break ;
    case 'N' :
        break ;
    case 'o':
        if (_.fout != 1)
            close(_.fout);
        _.fout = open(arg, O_WRONLY | O_CREAT | O_BINARY);
        break;
    case 'r' :
        break ;
    case 's' :
        _.suffix = arg;
        break ;
    case 'q' :
        _.verbose = 0;
        break ;
    case 'v' :
        _.verbose = 2;
        break ;
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :
        _.level = opt - '0';
        break ;
    }
}

int gzip_main(void *cfg, char *path)
{
    int lg, wr;
    gzFile gzfd;
    char buf[BUF_SZ];
    int fin = strcmp(path, "-") ? open(path, O_RDONLY | O_BINARY) : 0;
    if (fin == -1) {
        fprintf(stderr, "Unable to open file %s\n", path);
        return -1;
    }

    if (_.compress) {
        // Compress
        gzfd = gzdopen(_.fout, "w");
        do {
            lg = read(fin, buf, BUF_SZ);
            if (lg == 0)
                return -1;
            wr = gzwrite(gzfd, buf, lg);
            if (wr == 0)
                return -1;
        } while (lg != 0);
        gzflush(gzfd, Z_FINISH);
        gzclose(gzfd);
        if (fin != 0)
            close(fin);
    } else {
        // Decompress
        gzfd = gzdopen(fin, "r");
        do {
            lg = gzread(gzfd, buf, BUF_SZ);
            if (lg == 0)
                return -1;
            wr = write(_.fout, buf, lg);
            if (wr == 0)
                return -1;
        } while (lg != 0);
        gzclose(gzfd);
        if (_.fout != 1)
            close(_.fout);
    }
    return 0;
}


int main(int argc, char **argv)
{
    memset(&_, 0, sizeof(_));
    _.verbose = 1;
    _.suffix = "gz";
    _.compress = true;
    _.fout = 1;

    int n = arg_parse(argc, argv, gzip_param, NULL, options, usages);
    if (n == 0) {
        fprintf(stderr, "%s: Missing operands\n", argv[0]);
        return -1;
    }

    return arg_names(argc, argv, gzip_main, NULL, options);
}
