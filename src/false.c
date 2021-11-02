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
    END_OPTION("Exit with a status code indicating failure.")
};

char *usages[] = {
    "[ignored-arguments]",
    "OPTION",
    NULL,
};

int main(int argc, char **argv)
{
    for (int o = 1; o < argc; ++o) {
        if (strcmp(argv[o], "--help") == 0) {
            arg_usage(argv[0], options, usages);
            exit(0);
        }
        else if (strcmp(argv[o], "--version") == 0) {
            arg_version(argv[0]);
            exit(0);
        }
    }

    return -1;
}
