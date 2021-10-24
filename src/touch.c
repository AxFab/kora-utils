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
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

opt_t options[] = {
    OPTION('a', NULL, "Update only the access date"),
    OPTION('m', NULL, "Update only the modified date"),
    OPTION('c', NULL, "Do not create the file if it doens't exists"),
    END_OPTION("Update the access and modification time of a file.")
};

char *usages[] = {
    "[OPTION] [FILE]...",
    NULL,
};

char *__program;

struct params {
    int mode;
    int create;
} _;

void touch_parse_arg(void *p, unsigned char opt)
{
    switch(opt) {
    case 'a':
        _.mode = 1;
        break;
    case 'm':
        _.mode = 2;
        break;
    case 'c':
        _.create = 0;
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

int main(int argc, char **argv)
{
    __program = argv[0];
    time_t now = time(NULL);
    struct timeval clock;
    clock.tv_sec = now;
    // clock.tv_nsec = 0;
    _.mode = 3;
    _.create = 1;

    int o, n;
    n = arg_parse(argc, argv, (parsa_t)touch_parse_arg, NULL, options);
    if (n == 0) {
        fprintf(stderr, "Missing file operand\n");
        return 1;
    }

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;

        char *filename = argv[o];
        struct stat fstat;
        struct timeval times[2];

        int ret = stat(filename, &fstat);
        if (ret) {
            if (_.create == 0) {
                fprintf(stderr, "No such file %s\n", filename);
                return 1;
            }

            FILE *fp = fopen(filename, "a");
            if (fp == NULL) {
                fprintf(stderr, "Unable to create file %s\n", filename);
                return 1;
            }
            fclose(fp);
            continue;
        }

        if (_.mode & 1) {
            times[0].tv_sec = clock.tv_sec;
            // times[0].tv_nsec = clock.tv_nsec;
        } else {
            times[0].tv_sec = fstat.st_atime; //.tv_sec;
            // times[0].tv_nsec = fstat.st_atime.tv_nsec;
        }

        if (_.mode & 2) {
            times[1].tv_sec = clock.tv_sec;
            // times[1].tv_nsec = clock.tv_nsec;
        } else {
            times[1].tv_sec = fstat.st_mtime; //.tv_sec;
            // times[1].tv_nsec = fstat.st_mtime.tv_nsec;
        }

        ret = utimes(filename, times);
        if (ret) {
            fprintf(stderr, "Unable to update meta-data %s\n", filename);
            return 1;
        }
    }

    return 0;
}
