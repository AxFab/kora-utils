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
#include <fcntl.h>
#include <unistd.h>

#define BUF_SZ 512

opt_t options[] = {
    OPTION('L', NULL, "Show the logical location (default)"),
    OPTION('P', NULL, "Show the physical location, without symbolic links"),
    END_OPTION("Print the name of the current working directory.")
};

char *usages[] = {
    "[OPTION]",
    NULL,
};

int main(int argc, char **argv)
{
    int i;
    int physical = 0;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-' || argv[i][0] == '\0')
            continue;
        unsigned char *arg = argv[i][1] == '-' ? arg_long(&argv[i][2], options) : (unsigned char *)&argv[i][1];
        for (; *arg; ++arg) {
            switch (*arg) {
            case 'L' :
                physical = 0;
                break;
            case 'P' :
                physical = 1;
                break;
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

    char buffer[4096] = {0};
    if (physical == 0) {
        char *t = getenv("PWD");
        if (t)
            strncpy(buffer, t, 4096);
    }

    if (buffer[0] == 0) {
        if (!getcwd(buffer, 4096))
            return 1;
    }

    puts(buffer);
    return 0;
}
