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
    END_OPTION("Exit with a status code indicating sucess.")
};

char *usages[] = {
    "[ignored-arguments]",
    "OPTION",
    NULL,
};

int main(int argc, char **argv)
{
    int o, n = argc;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] != '-') {
            n = o;
            break;
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

    char msg[4096];
	strcpy(msg, "!");
        for (o = n; o < argc; ++o) {
	    strcat(msg, argv[o]);
	    strcat(msg, " ");
	}
	strcat(msg, "%s\n");
    
    char buf[4096];
    for (;;) {
        int ret = scanf("%s", buf);
	if (ret <= 0)
	    break;
	printf(msg, buf);
    }
    return 0;
}

