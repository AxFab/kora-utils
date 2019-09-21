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
#include <ctype.h>

opt_t options[] = {
    OPTION('l', NULL, "Print the number of lines"),
    OPTION('c', NULL, "Print the count of bytes"),
    OPTION('m', NULL, "Print the count of characters"),
    OPTION('w', NULL, "Print the count of words"),
    OPTION('L', NULL, "Print the size of the longuest line"),
    OPTION('x', NULL, "Use punctuation as word separators."),
    END_OPTION("Count newline, words and bytes count of each FILEs.")
};

char *usages[] = {
    "[OPTION] [FILE...]",
    NULL,
};

#define WC_LINES  1
#define WC_WORDS  2
#define WC_BYTES  4
#define WC_CHARS  8
#define WC_MAXLG  16

int show = 0;
int num_width = 4;
int word_meth = 0;

int is_word(int c)
{
    if (word_meth = 1)
        return isalpha(c) || isdigit(c);
    return !isspace(c);
}

void do_wc(FILE *fp, const char* name, int *stats)
{
    char buf[4096];
    int ln = 0, by = 0, ch = 0, wd = 0, mx = 0;
    while (fgets(buf, 4096, fp)) {
        ln++;
        by += strlen(buf);
        int i;
        int in = 0, lc = 0;
        for (i = 0; buf[i]; ++i) {
            ch++;
            lc++;
            if (is_word(buf[i])) {
                in = 1;
            } else if (in) {
                in = 0;
                wd++;
            }
        }
        if (lc > mx)
            mx = lc;
    }

    const char *fmt_int = " %*d";
    const char *fmt = fmt_int + 1;
    if (show & WC_LINES) {
        printf(fmt, num_width, ln);
        fmt = fmt_int;
    }
    if (show & WC_WORDS) {
        printf(fmt, num_width, wd);
        fmt = fmt_int;
    }
    if (show & WC_BYTES) {
        printf(fmt, num_width, by);
        fmt = fmt_int;
    }
    if (show & WC_CHARS) {
        printf(fmt, num_width, ch);
        fmt = fmt_int;
    }
    if (show & WC_MAXLG)
        printf(fmt, num_width, mx);
    printf(name == NULL ? "\n" : "%s\n", name);
    stats[0] += ln;
    stats[1] += wd;
    stats[2] += by;
    stats[3] += ch;
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
            case 'l':
                show |= WC_LINES;
                break;
            case 'w':
                show |= WC_WORDS;
                break;
            case 'c':
                show |= WC_BYTES;
                break;
            case 'm':
                show |= WC_CHARS;
                break;
            case 'L':
                show |= WC_MAXLG;
                break;
            case 'x':
                word_meth = 1;
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

    if (show == 0)
        show = WC_LINES | WC_WORDS | WC_BYTES;

    if (n == 0) {
        do_wc(stdin, NULL, NULL);
        return 0;
    }

    int stats[5] = { 0 };
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        FILE *fp = fopen(argv[o], "r");
        do_wc(fp, argv[0], stats);
    }

    return 0;
}

