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

int main(int argc, char **argv)
{
    int n =arg_parse(argc, argv, NULL, NULL, options, usages);

    char buf[4096];
    if (n == 0)
        strcpy(buf, "y");
    else {
        int len = snprintf(buf, 4096, "%s", argv[1]);
        for (int o = 2; o < argc; ++o)
            len += snprintf(&buf[len], 4096 - len, " %s", argv[o]);
    }

    for (;;)
        printf("%s\n", buf);
    return 0;
}
