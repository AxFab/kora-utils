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
    OPTION('l', NULL, "Produce long format output for each users"),
    OPTION('b', NULL, "Omit the users home directory and shell in long format."),
    OPTION('s', NULL, "Do short format output, this is the default."),
    // #f, omit the line of columns in short format
    //  w,i,q omit fullname, remote host, idle time
    END_OPTION("Display user information.")
};

char *usages[] = {
    "[ignored-arguments]",
    "OPTION",
    NULL,
};

void display_long_field(const char *nm, const char *val)
{
    printf("%s:  %-*s", nm, 30 - strlen(nm), val);
    printf("\n");
}

struct user {
    // uuid
    // email
    // home
    // shell
    char login[128];
    char fullname[128];
    char ssid[32];
    // time_t since;
    int where;
};

void display_long(int uid)
{
    display_long_field("Uid", "5774");
    display_long_field("Username", "fab");
    display_long_field("Realname", "Fabien");
    display_long_field("Email", "fabien.bavent@gmail.com");
    display_long_field("Home", "/home/fab");
    display_long_field("Shell", "/usr/bin/krish");
    display_long_field("Origin", "localhost");
    printf("\n");
}

void display_header()
{
    printf("%-8s  ", "Username");
    printf("%-12s  ", "Fullname");
    printf("%-6s  ", "SSID");
    printf("%-4s  ", "Idle");
    printf("%-16s  ", "Since");
    printf("%-12s  ", "Remote");
    printf("\n");
}
void display_short()
{
    printf("%-8s  ", "fabien");
    printf("%-12s  ", "Fabien");
    printf("%-6s  ", "Dsk/8");
    printf("%4s  ", "4h");
    printf("%-16s  ", "2019-07-01 11:34");
    printf("%-12s  ", "local");
    printf("\n");
}

int main(int argc, char **argv)
{
    int o, n = 0;
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
            case OPT_HELP: // --help
                arg_usage(argv[0], options, usages);
                return 0;
            case OPT_VERS: // --version
                arg_version(argv[0]);
                return 0;
            }
        }
    }

    display_long(0);


    display_header();
    display_short();
    // display_short();
    // display_short();

    // FILE *info = fopen("/etc/passwd", "r");
    // if (info == NULL)
    //     return;
    // char buf[512];
    // while (fgets(buf, 512, info)) {
    //     char * login = strtok(buf, ":\t\n");
    //     char * passwd = strtok(NULL, ":\t\n");
    //     char * uid = strtok(NULL, ":\t\n");
    //     char * gid = strtok(NULL, ":\t\n");
    //     char * name = strtok(NULL, ":\t\n");
    //     printf("| %-16s\t%-16s\t%s\n", login, name, uid);
    // }
    return 0;
}

