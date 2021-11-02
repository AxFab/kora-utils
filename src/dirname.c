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

opt_t options[] = {
    OPTION('z', "zero", "Finish all line by NULL unstead of EOL"),
    END_OPTION("Strip non-directory suffix from PATH. ")
};

char *usages[] = {
    "OPTION... NAME...",
    NULL,
};

struct {
    char eol;
} _;


void dirname_parse_arg(void *param, int opt, char *arg)
{
    (void)param;
    switch (opt) {
    case 'z': // --zero
        _.eol = '\0';
        break;
    }
}

int do_dirname(void *param, char *path)
{
    int lg = strlen(path);
    if (lg > 0 && path[lg - 1] == '/')
        path[lg - 1] = '\0';

    char *name = strrchr(path, '/');
    if (name != NULL)
        name[0] = '\0';

    printf("%s%c", path, _.eol);
    return 0;
}

int main(int argc, char **argv)
{
    // _.multiple = 0;
    _.eol = '\n';

    int n = arg_parse(argc, argv, dirname_parse_arg, NULL, options, usages);
    if (n == 0) {
        fprintf(stderr, "%s: except an operand\n" HELP, argv[0], argv[0]);
        return -1;
    }

    return arg_names(argc, argv, do_dirname, NULL, options);
}
