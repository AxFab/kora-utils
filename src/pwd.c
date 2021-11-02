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
    OPTION('L', NULL, "Show the logical location (default)"),
    OPTION('P', NULL, "Show the physical location, without symbolic links"),
    END_OPTION("Print the name of the current working directory.")
};

char *usages[] = {
    "[OPTION]",
    NULL,
};

int physical = 0;

void pwd_parse(void *cfg, int opt, char *arg)
{
    switch (*arg) {
    case 'L' :
        physical = 0;
        break;
    case 'P' :
        physical = 1;
        break;
    }
}

int main(int argc, char **argv)
{
    arg_parse(argc, argv, pwd_parse, NULL, options, usages);

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
