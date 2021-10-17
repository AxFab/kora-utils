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

int main(int argc, char **argv)
{
    // int multiple = 0;
    char *name = NULL;

    int o, n = 0;
    char eol = '\n';
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] != '-') {
            n++;
            continue;
        }

        unsigned char *opt = (unsigned char *)&argv[o][1];
        if (*opt == '-')
            opt = arg_long(&argv[o][2], options);
        for (; *opt; ++opt) {
            switch (*opt) {
            case 'z': // --zero
                eol = '\0';
                break;
            case OPT_HELP: // --help
                arg_usage(argv[0], options, usages);
                return 0;
            case OPT_VERS: // --version
                arg_version(argv[0]);
                return 0;
            }
        }
    }

    if (n == 0) {
        fprintf(stderr, "%s: except an operand\n" HELP, argv[0], argv[0]);
        return -1;
    }

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;

        int lg = strlen(argv[o]);
        if (lg > 0 && argv[o][lg - 1] == '/')
            argv[o][lg - 1] = '\0';

        name = strrchr(argv[o], '/');
        if (name != NULL)
            name[0] = '\0';

        printf("%s%c", argv[o], eol);
    }
    return 0;
}
