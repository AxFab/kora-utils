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
#include <stdlib.h>
#include <time.h>

opt_t options[] = {
    OPTION('1', NULL, "Display on 1 column, like -C 1"),
    OPTION('3', NULL, "Display on 3 columns, like -C 3"),
    OPTION('R', "rows", "Display several months, print x row"),
    OPTION('C', "cols", "Display several months, print x column"),
    // OPTION('j', NULL, "Show day of the year instead of day of month"),
    // OPTION('y', NULL, "Display the all year"),
    END_OPTION("Display monthly calandar.")
};

char *usages[] = {
    "[OPTION]",
    NULL,
};

const char *months[] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    "",
};

int days_on_month(int m, int y)
{
    ++m;
    if (m == 2)
        return 28;
    int r = m % 2;
    if (m > 7)
        r = 1 - r;
    return r ? 31 : 30;
}

int wday_of_first(int m, int y)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mon = m;
    tm.tm_year = y;
    tm.tm_mday = 1;
    time_t ti = mktime(&tm);
    gmtime_r(&ti, &tm);
    // printf("Date M%d, Y%d, D%d, W%d\n", tm.tm_mon, tm.tm_year, tm.tm_mday, tm.tm_wday);
    int w = - tm.tm_wday;
    return w < -6 ? w + 8 : w + 1;
}


void display_month(int m, int y)
{
    while (m >= 12) {
        m -= 12;
        y++;
    }
    char month[32];
    snprintf(month, 32, "%s %d", months[m], y);
    int ds = strlen(month);
    ds = (21 - ds) / 2 + ds;
    printf("%*s%*s", ds, month, 21 - ds, "");
}

void display_wdays()
{
    printf("Su Mo Tu We Th Fr Sa ");
}

void display_week(int w, int d, int m, int t)
{
    int i;
    d += w * 7;
    for (i = 0; i < 7; ++i) {
        if (i + d > 0 && i + d <= m) {
            if (i + d == t)
                printf("\033[30;47m%2d\033[0m ", i + d);
            else
                printf("%2d ", i + d);
        } else
            printf("   ");
    }
}

time_t parse_date(const char *str)
{
    int m = 0;
    int y = 0;
    if (strlen(str) == 7 && str[4] == '-') { // "[0-9]{4}-[0-9]{2}"
        m = strtol(&str[5], NULL, 10) - 1;
        y = strtol(str, NULL, 10) - 1900;
    } else
        return 0;

    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_mon = m;
    tm.tm_year = y;
    tm.tm_mday = 1;
    return mktime(&tm);
}

int main(int argc, char **argv)
{
    int c, C = 1;
    int r, R = 1;
    int o, n = 0;

    time_t today = time(NULL);
    time_t showdate = time(NULL);

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] != '-') {
            n++;
            continue;
        }

        unsigned char *opt = (unsigned char *)&argv[o][1];
        if (*opt == '-')
            opt = arg_long(&argv[o][2], options);
        for (; *opt; ++opt) {
            switch (*opt) {
            case '1':
                C = 1;
                break;
            case '3':
                C = 3;
                break;
            case 'R': // --rows
                R = strtol(argv[o + 1], NULL, 10);
                argv[o + 1] = "-";
                break;
            case 'C': // --cols
                C = strtol(argv[o + 1], NULL, 10);
                argv[o + 1] = "-";
                break;
            case 'd':
                showdate = parse_date(argv[o + 1]);
                argv[o + 1] = "-";
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

    if (showdate == 0) {
        printf("Invalid date!\n");
        return -1;
    }

    struct tm tdd;
    gmtime_r(&today, &tdd);

    struct tm tmd;
    gmtime_r(&showdate, &tmd);

    int i;

    // printf("Date M%d, Y%d, D%d, W%d\n", tmd.tm_mon, tmd.tm_year, tmd.tm_mday, tmd.tm_wday);

    int w = tmd.tm_wday - tmd.tm_mday % 7;
    int m = tmd.tm_mon;
    int y = tmd.tm_year;

    for (r = 0; r < R; ++r) {
        for (c = 0; c < C; ++c) {
            if (c != 0)
                printf("  ");
            display_month(m + c + r * C, y + 1900);
        }
        printf("\n");

        for (c = 0; c < C; ++c) {
            if (c != 0)
                printf("  ");
            display_wdays();
        }
        printf("\n");

        for (i = 0; i < 6; ++i) {
            for (c = 0; c < C; ++c) {
                w = wday_of_first(m + c + r * C, y);
                if (c != 0)
                    printf("  ");
                display_week(i, w, days_on_month(m + c + r * C, y), m + c + r * C == tdd.tm_mon ? tdd.tm_mday : -1);
            }
            printf("\n");
        }
        printf("\n");
    }

    return 0;
}

