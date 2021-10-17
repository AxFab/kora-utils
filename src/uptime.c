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
#include <time.h>

opt_t options[] = {
    OPTION('p', "pretty", "Show pretty format"),
    OPTION('s', "since", "System up since"),
    END_OPTION("Show the time since the system is up.")
};

char *usages[] = {
    "[ignored-arguments]",
    "OPTION",
    NULL,
};

int main(int argc, char **argv)
{
    char mode = 'r';

    int o;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] != '-')
            break;

        unsigned char *opt = (unsigned char *)&argv[o][1];
        if (*opt == '-')
            opt = arg_long(&argv[o][2], options);
        for (; *opt; ++opt) {
            switch (*opt) {
            case 'p':
            case 's':
                mode = *opt;
                break;
            case OPT_HELP: // --help
                arg_usage(argv[0], options, usages);
                return 0;
            case OPT_VERS: // --version
                arg_version(argv[0]);
                return 0;
            }
        }
    }


    time_t nowT = time(NULL);
    time_t upT = time(NULL);

    char buffer[128];
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp) {
        fread(buffer, 128, 1, fp);
        upT -= strtol(buffer, NULL, 10);
        fclose(fp);
    }

    struct tm now;
    gmtime_r(&nowT, &now);
    now.tm_year += 1900;

    struct tm up;
    gmtime_r(&upT, &up);
    up.tm_year += 1900;

    int diff = nowT - upT;
    int dDay = diff / 3600 / 24;
    int dHours = (diff / 3600) % 24;
    int dMin = (diff / 60) % 60;
    int users = 1; // TODO

    if (mode == 's') {
        printf(" %04d-%02d-%02d %02d:%02d:%02d\n",
               up.tm_year, up.tm_mon, up.tm_mday,
               up.tm_hour, up.tm_min, up.tm_sec);
    } else if (mode == 'p') {
        if (dDay > 0)
            printf(" Up %d days, %d hours and %d minutes\n", dDay, dHours, dMin);
        else
            printf(" Up %d hours and %d minutes\n", dHours, dMin);
    } else {
        if (dDay > 0)
            printf(" %02d:%02d:%02d  up %d.%02d:%02d, %d users\n",
                   now.tm_hour, now.tm_min, now.tm_sec,
                   dDay, dHours, dMin, users);
        else
            printf(" %02d:%02d:%02d  up %02d:%02d, %d users\n",
                   now.tm_hour, now.tm_min, now.tm_sec,
                   dHours, dMin, users);
    }

    return 0;
}
