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

#define BUF_SZ 512

opt_t options[] = {
    OPTION('c', "stdout", "Write output on standard output ; keep original files unchanged (default)"),
    OPTION('d', "decompress", "Decompress input files"),
    // OPTION('f', "force", "..."),
    // OPTION('l', "list", "Print info for each compressed file"),
    // OPTION('n', "no-name", "..."),
    // OPTION('N', "name", "..."),
    OPTION('o', "output", "Write output on a file"),
    // OPTION('s', "suffix", "..."),
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


int main(int argc, char **argv)
{
    int i;
    int level;
    int verbose = 1;
    char *suffix = "gz";
    bool compress = true;
    int fout = 1;
    int lg;
    char buf[BUF_SZ];
    gzFile gzfd;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-' || argv[i][0] == '\0')
            continue;
        char *arg = argv[i][1] == '-' ? arg_long(&argv[i][2], options) : &argv[i][1];
        for (; *arg; ++arg) {
            switch (*arg) {
            case 'c':
                if (fout != 1)
                    close(fout);
                fout = 1;
                break ;
            case 'd' :
                compress = false;
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
                return 1;
            case 'n' :
                break ;
            case 'N' :
                break ;
            case 'o':
                if (fout != 1)
                    close(fout);
                fout = open(argv[++i], O_WRONLY | O_CREAT | O_BINARY);
                break;
            case 'r' :
                break ;
            case 's' :
                suffix = argv[++i];
                break ;
            case 'q' :
                verbose = 0;
                break ;
            case 'v' :
                verbose = 2;
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
                level = *arg - '0';
                break ;
            case OPT_HELP :
                arg_usage(argv[0], options, usages) ;
                return 0;
            case OPT_VERS :
                arg_version(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Option -%c non recognized.\n" HELP, *arg, argv[0]);
                return 1;
            }
        }
    }


    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            if (argv[i][1] == 'o')
                ++i;
            continue;
        }
        int wr;
        char *path = argv[i];
        // char *path_out = argv[i];
        int fin = strcmp(path, "-") ? open(path, O_RDONLY | O_BINARY) : 0;
        if (fin == -1) {
            fprintf(stderr, "Unable to open file %s\n", path);
            return 1;
        }

        if (compress) {
            // Compress
            gzfd = gzdopen(fout, "w");
            do {
                lg = read(fin, buf, BUF_SZ);
                if (lg != 0)
                    wr = gzwrite(gzfd, buf, lg);
            } while (lg != 0);
            gzflush(gzfd, Z_FINISH);
            // fflush(fout);
            gzclose(gzfd);
            if (fin != 0)
                close(fin);
        } else {
            // Decompress
            gzfd = gzdopen(fin, "r");
            do {
                lg = gzread(gzfd, buf, BUF_SZ);
                if (lg != 0)
                    wr = write(fout, buf, lg);
            } while (lg != 0);
            // flush(fout);
            gzclose(gzfd);
            if (fout != 1)
                close(fout);
        }
    }
    return 0;
}
