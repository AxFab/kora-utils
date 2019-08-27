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
#include "utils.h"

opt_t options[] = {
    OPTION('a', "all", "Print all information"),
    OPTION('s', NULL, "Print the kernel name (default)"),
    OPTION('n', NULL, "Print the machine hostname"),
    OPTION('r', NULL, "Print the kernel release"),
    OPTION('v', NULL, "Print the kernel version"),
    OPTION('m', NULL, "Print the architecture name"),
    OPTION('p', NULL, "Print the processor type"),
    OPTION('i', NULL, "Print hardware platform"),
    OPTION('o', NULL, "Print the OS name"),
    END_OPTION("Display system information.")
};

char *usages[] = {
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

    printf("%s ", "Kora");
    char buf[64];
    gethostname(buf, 64);
    printf("%s ", buf);
    printf("%s ", "0.1.1+");
    printf("%s ", "#0.SP");
    printf("%s ", "arm");
    printf("%s ", "armv7l");
    printf("%s ", "KoraOS");
    printf("\n");
    return 0;
}

