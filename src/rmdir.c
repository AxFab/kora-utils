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

void rmdir_parse_args(void* param, int opt, char *arg)
{
    switch(opt) {
    }
}

int do_rmdir(void* param, char *path)
{
    // TODO
    return -1;
}

int main(int argc, char** argv)
{
    int n = arg_parse(argc, argv, rmdir_parse_args, NULL, options, usages);
    if (n == 0) {
        fprintf(stderr, "Missing operand.\n");
        arg_usage(argv[0], options, usages);
        return -1;
    }

    return arg_names(argc, argv, do_rmdir, NULL, options);
}
