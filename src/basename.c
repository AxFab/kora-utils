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
    OPTION('a', "multiple", "Accept several operands as PATH"),
    OPTION_A('s', "suffix=SUFFIX", "Use the next argument as SUFFIX, to remove"),
    OPTION('z', "zero", "Finish all line by NULL unstead of EOL"),
    END_OPTION("Print PATH without the parent directoies of the last component. "
               "If requested and find, SUFFIX can be removed from the end of the name.")
};

char *usages[] = {
    "NAME [SUFFIX]...",
    "OPTION... NAME...",
    NULL,
};

struct {
    int multiple;
    int ignore;
    const char *suffix;
    char eol;
} _;

void basename_parse_arg(void *param, int opt, char *arg)
{
    (void)param;
    switch (opt) {
    case 'a': // --multiple
        _.multiple = 1;
        break;
    case 's': // --suffix
        _.multiple = 1;
        _.suffix = arg;
        break;
    case 'z': // --zero
        _.eol = '\0';
        break;
    }
}

int do_basename(void *param, char *path)
{
    if (_.ignore)
        return 0;

    int lg = strlen(path);
    if (lg > 0 && path[lg - 1] == '/')
        path[lg - 1] = '\0';

    char *name = strrchr(path, '/');
    if (name == NULL)
        name = path;
    else
        name++;

    if (_.suffix != NULL) {
        int off = strlen(name) - strlen(_.suffix);
        if (strcmp(&name[off], _.suffix) == 0)
            name[off] = '\0';
    }

    printf("%s%c", name, _.eol);
    if (_.multiple == 0)
        _.ignore = 1;

    return 0;
}

int main(int argc, char **argv)
{
    memset(&_, 0, sizeof(_));
    _.eol = '\n';

    int n = arg_parse(argc, argv, basename_parse_arg, NULL, options, usages);

    if (n == 0) {
        fprintf(stderr, "%s: except an operand\n" HELP, argv[0], argv[0]);
        return -1;
    }

    if (n >= 2 && _.multiple == 0)
        _.suffix = arg_index(argc, argv, 2, options);
    return arg_names(argc, argv, do_basename, NULL, options);
}
