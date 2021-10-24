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
    END_OPTION("Output reapeteadly STRING, or 'y'.")
};

char *usages[] = {
    "[STRING]",
    NULL,
};

char* __program;

void yes_parse_args(void* cfg, unsigned char arg)
{
    switch (arg) {
    case OPT_HELP: // --help
        arg_usage(__program, options, usages);
        exit(0);
    case OPT_VERS: // --version
        arg_version(__program);
        exit(0);
    }
}

int main(int argc, char **argv)
{
    int o, n;
    __program = argv[0];
    n = arg_parse(argc, argv, yes_parse_args, NULL, options);

    char buf[4096];
    if (n == 0)
        strcpy(buf, "y");
    else {
        int len = snprintf(buf, 4096, "%s", argv[1]);
        for (o = 2; o < argc; ++o)
            len += snprintf(&buf[len], 4096 - len, " %s", argv[o]);
    }

    for (;;)
        printf("%s\n", buf);
    return 0;
}
