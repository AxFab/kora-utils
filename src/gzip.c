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
#include <kora/utils.h>
#include <zlib.h>

#define BUF_SZ 512

opt_t options[] = {
    OPTION('c', "stdout", "Write output on standard output ; keep original files unchanged"),
    OPTION('d', "decompress", "Decompress input files"),
    OPTION('f', "force", "..."),
    OPTION('l', "list", "Print info for each compressed file"),
    OPTION('L', "license", "Display the  gzip license and quit"),
    OPTION('n', "no-name", "..."),
    OPTION('N', "name", "..."),
    OPTION('s', "suffix", "..."),
    OPTION('q', "quiet", "Suppress all warnings"),
    OPTION('r', "recursive", "..."),
    OPTION('v', "verbose", "..."),
    OPTION('1', "fast", "..."),
    OPTION('9', "best", "..."),
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

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-' || argv[i][0] == '\0')
            continue;
        char *arg = argv[i][1] == '-' ? arg_long(&argv[i][2], options) : &argv[i][1];
        for (; *arg; ++arg) {
            switch (*arg) {
            case 'c' :
                break ;
            case 'd' :
                break;
            case 'f' :
                break ;
            case 'l' :
                break ;
            case 'L' :
                break;
            case 'n' :
                break ;
            case 'N' :
                break ;
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

    char buf[BUF_SZ] ;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1] != '\0')
            continue;
        char *path = argv[i];
        char *path_out = argv[i];
        int fd = strcmp(path, "-") ? open(path, oflg) : 1;
        if (fd == -1) {
            fprintf(stderr, "Unable to open file %s\n", path);
            return 1;
        }

        int fd_out = open(path_out, "w");

        // Compress
        gzFile gzfd = gzdopen(fd_out, "w");
        do {
            lg = read(fd, buf, BUF_SZ);
            gzwrite(gzfd, buf, lg);
        } while (lg != 0);
        gzclose(gzfd);

        // Decompress
        gzFile gzfd = gzdopen(fd, "r");
        do {
            lg = gzread(gzfd, buf, BUF_SZ);
            write(fd_out, buf, lg);
        } while (lg != 0);
        gzclose(gzfd);

        close(fd_out);
    }
    return 0;
}
