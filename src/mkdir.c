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
    OPTION('v', "verbose", "explain what is being done"),
    END_OPTION("...")
};

char* usages[] = {
    "[OPTION]... [FILE]...",
    NULL,
};

char* __program;

void mkdir_parse_args(void* param, unsigned char arg)
{
}

int main(int argc, char** argv)
{
    int o;
    __program = argv[0];

    int n = arg_parse(argc, argv, (parsa_t)mkdir_parse_args, &_, options);
    if (n == 0) {
        fprintf(stderr, "Missing operand.\n");
        arg_usage(__program, options, usages);
        return -1;
    }

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        // TODO
    }

    return -1;
}
