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
#include <kora/mcrs.h>

opt_t options[] = {
    OPTION('u', NULL, "Use unify format"),
    END_OPTION("Compare files line by line.")
};

char *usages[] = {
    "[OPTIONS] FILES...",
    NULL,
};


typedef struct ftxt ftxt_t;

struct ftxt {
    FILE *fp;
    int rows;
    char **row_cache;
};

ftxt_t *ftxt_open(FILE *fp)
{
    ftxt_t *fx = calloc(1, sizeof(ftxt_t));
    fx->fp = fp;
    char line[512];
    int r = 0;
    fx->row_cache = calloc(1000, sizeof(char *));
    while (fgets(line, 512, fp))
        fx->row_cache[r++] = strdup(line);
    fx->rows = r;
    // printf("Read file, %d lines\n", r);
    return fx;
}

int ftxt_shr(ftxt_t *fx, const char *ln, int start)
{
    for (; fx->row_cache[start]; ++start) {
        char *rw = fx->row_cache[start];
        if (strcmp(rw, ln) == 0)
            return start;
    }
    return -1;
}

char cP = '>';
char cM = '<';
int ctx = 0;
int tail = 9999;

void do_diff(FILE *fp1, FILE *fp2)
{
    ftxt_t *f1 = ftxt_open(fp1);
    ftxt_t *f2 = ftxt_open(fp2);
    int r1 = 0, r2 = 0, df1 = -1, df2 = -1;
    for (;;) {
        char *l1 = f1->row_cache[r1];
        char *l2 = f2->row_cache[r2];
        if (l1 == NULL || l2 == NULL)
            break;
        if (strcmp(l1, l2) == 0) {
            if (tail < ctx) {
                printf("  %s", l1);
                tail++;
                df1++;
                df2++;
            }
            ++r1;
            ++r2;
            continue;
        }
        // print context
        if (df1 != r1 || df2 != r2) {
            if (df1 + ctx < r1) {
                if (ctx == 0)
                    printf("%dc%d\n", r1 + 1 - ctx, r2 + 1 - ctx);
                else
                    printf("\033[36m@@ -%d,x +%d,x @@\033[0m\n", r1 + 1 - ctx, r2 + 1 - ctx);
            }
            if (ctx > 0) {
                int i = MAX(0, MAX(df1, r1 - ctx));
                for (; i < r1; ++i)
                    printf("  %s", f1->row_cache[i]);
            }
            df1 = r1;
            df2 = r2;
        } else if (ctx == 0)
            printf("---\n");

        tail = 0;
        // Search the line on target
        int nx = ftxt_shr(f2, l1, r2);
        if (nx >= 0) {
            while (r2 < nx) {
                printf("\033[32m%c %s\033[0m", cP, l2);
                l2 = f2->row_cache[++r2];
                df2++;
            }
            continue;
        }

        // Search the line on origin
        nx = ftxt_shr(f1, l2, r1);
        if (nx >= 0) {
            while (r1 < nx) {
                printf("\033[31m%c %s\033[0m", cM, l1);
                l1 = f1->row_cache[++r1];
                df1++;
            }
            continue;
        }
        printf("\033[31m%c %s\033[0m", cM, l1);
        ++r1;
        df1++;
    }
}


int main(int argc, char **argv)
{
    char *path[2];
    int o, n = 0;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] != '-') {
            if (n > 1)
                fprintf(stderr, "Extra operand: %s\n", argv[o]);
            else
                path[n] = argv[o];
            n++;
            continue;
        }

        unsigned char *opt = (unsigned char *)&argv[o][1];
        if (*opt == '-')
            opt = arg_long(&argv[o][2], options);
        for (; *opt; ++opt) {
            switch (*opt) {
            case 'u':
                ctx = 3;
                cP = '+';
                cM = '-';
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

    if (n > 2)
        return -1;
    else if (n < 2) {
        fprintf(stderr, "Missing operand\n");
        return -1;
    }


    if (ctx != 0) {
        printf("\033[1m--- %s\033[0m\n", path[0]);
        printf("\033[1m+++ %s\033[0m\n", path[1]);
    }
    FILE *f1 = fopen(path[0], "r");
    FILE *f2 = fopen(path[1], "r");
    do_diff(f1, f2);
    return 0;
}

