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
#include <threads.h>

opt_t options[] = {
    END_OPTION("Pause for a number of seconds. Adding a suffix allow to change the time unit.")
};

char *usages[] = {
    "NUMBER[SUFFIX]...",
    "OPTION",
    NULL,
};

int main(int argc, char **argv)
{
    int o, n = 0;
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
            case OPT_HELP: // --help
                arg_usage(argv[0], options, usages);
                return 0;
            case OPT_VERS: // --version
                arg_version(argv[0]);
                return 0;
            }
        }
    }

    if (argc < 2) {
        fprintf(stderr, "Missing operand\n");
        return 1;
    }

    char *p;
    struct timespec timeout;
    timeout.tv_sec = strtol(argv[1], &p, 10);
    timeout.tv_nsec = 0;

    if (argv[1] == p) {
        fprintf(stderr, "Bad operand: %s\n", argv[1]);
        return 1;
    }

    thrd_sleep(&timeout, NULL);
    return 0;
}
