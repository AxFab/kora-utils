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
#include <fcntl.h>
#include <unistd.h>

#define BUF_SZ 512

opt_t options[] = {
    OPTION('A', "show-all", "Equivalent to options -vET"),
    OPTION('b', "number-nonblank", "Number non blank lines"),
    OPTION('e', NULL, "Equivalent to options -vE"),
    OPTION('E', "show-ends", "Display a $ character at the end of each line"),
    OPTION('n', "number", "Number all lines"),
    OPTION('s', "squeeze-blank", "Delete repeated empty lines"),
    OPTION('t', NULL, "Equivalent to options -vT"),
    OPTION('T', "show-tabs", "Display tab character as ^I"),
    OPTION('v', "show-non-printing", "Use ^ and M- notations for control characters, excepted for EOL and TAB"),
    END_OPTION("Concatenate FILEs or standard input into standard output.")
};

char *usages[] = {
    "OPTION... FILE...",
    NULL,
};

char *notations[] = {
    "?", "?", "?", "?", "?", "?", "?", "?",
    "", "^I", "$\n", "?", "?", "M-", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
};
char notations_lg[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

struct {
    int blanks;
    int numbers;
    int squeeze;
    int row;
    char last;
} _;

void cat_parse_args(void *param, int opt, char *arg)
{
    switch (opt) {
    case 'A' :
        _.blanks |= 7;
        break ;
    case 'b' :
        _.numbers ++;
        break;
    case 'e' :
        _.blanks |= 5;
        break ;
    case 'E' :
        _.blanks |= 4;
        break ;
    case 'n' :
        _.numbers = 2;
        break;
    case 's' :
        _.squeeze = 1;
        break ;
    case 't' :
        _.blanks |= 3;
        break ;
    case 'T' :
        _.blanks |= 2;
        break ;
    case 'v' :
        _.blanks |= 1;
        break ;
    }
}

int do_cat(void *param, char *path)
{
    char buf[BUF_SZ];
    int fd = strcmp(path, "-") ? open(path, O_RDONLY) : 0;
    if (fd == -1) {
        fprintf(stderr, "Unable to open file %s\n", path);
        return -1;
    }

    for (;;) {
        int lg = read(fd, buf, BUF_SZ);
        if (lg == 0)
            break ;
        if (lg < 0)
            return -1;

        if (_.numbers + _.blanks + _.squeeze == 0) {
            if (write(1, buf, lg) <= 0)
                return -1;
            continue;
        }

        int st = 0;
        while (st < lg) {
            if (_.last == '\n' && (_.numbers > 2 || (_.numbers && buf[st] != '\n')))
                dprintf(1, "%6d  ", ++_.row);
            if (_.squeeze && _.last == '\n') {
                // TODO - Erase until only one blank line!
            }

            int wr = st;
            while ((buf[wr] < 0 || buf[wr] >= 0x20) && wr < lg)
                wr++;
            if (st != wr) {
                if (write(1, & buf[st], wr - st) <= 0)
                    return -1;
            }

            if (wr == lg) {
                //
                break;
            }

            switch (buf[wr]) {
            case '\n' :
                if (_.blanks & 4)
                    write(1, notations[(int)buf[wr]], notations_lg[(int)buf[wr]]);
                else
                    write(1, &buf[wr], 1);
                break;
            case '\t' :
                if (_.blanks & 2)
                    write(1, notations[(int)buf[wr]], notations_lg[(int)buf[wr]]);
                else
                    write(1, &buf[wr], 1);
                break;
            default:
                if (_.blanks & 1)
                    write(1, notations[(int)buf[wr]], notations_lg[(int)buf[wr]]);
                else
                    write(1, &buf[wr], 1);
                break;
            }
            _.last = buf[wr];
            st = wr + 1;
        }
    }
    return 0;
}


int main(int argc, char **argv)
{
    memset(&_, 0, sizeof(_));
    _.last = '\n';
    arg_parse(argc, argv, cat_parse_args, NULL, options, usages);
    return arg_names(argc, argv, do_cat, NULL, options);
}
