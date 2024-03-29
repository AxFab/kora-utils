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
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcrs.h"

#define BUF_SZ 512

opt_t options[] = {
    OPTION_A('r', "range", ""),
    END_OPTION("Dump content of a binary file.")
};

char *usages[] = {
    "OPTION... FILE...",
    NULL,
};

struct {
    __off_t s_range;
    __off_t e_range;
} _;

void hd_parse(void *cfg, int opt, char *arg)
{
    char *sv_str;
    switch (opt) {
    case 'r':
        _.s_range = strtol(arg, &sv_str, 10);
        if (*sv_str == '-')
            _.e_range = strtol(&sv_str[1], NULL, 10);
        break;
    }
}

int hd_main(void *cfg, char *path)
{
    char buf[BUF_SZ];
    int fd = strcmp(path, "-") ? open(path, O_RDONLY) : 0;
    if (fd == -1) {
        fprintf(stderr, "Unable to open file %s\n", path);
        return -1;
    }

    if (_.s_range != 0) {
        lseek(fd, _.s_range, SEEK_CUR);
    }
    __off_t addr = _.s_range;
    int i, by = 0;
    bool prev_zeros = false;
    bool skip_zeros = false;
    for (;;) {
        int lg = read(fd, &buf[by], BUF_SZ - by);
        if (lg == 0)
            break;
        if (lg < 0)
            return -1;

        lg += by;
        by = 0;
        while (lg >= 16) {
            uint32_t* pz = (uint32_t*)&buf[by];
            if (pz[0] == 0 && pz[1] == 0 && pz[2] == 0 && pz[3] == 0) {
                if (prev_zeros) {
                    skip_zeros = true;
                    by += 16;
                    lg -= 16;
                    addr += 16;
                    continue;
                }
                prev_zeros = true;
            }
            else {
                prev_zeros = false;
                if (skip_zeros) {
                    printf("%08llux    *** ***\n", addr - 16LLU);
                    skip_zeros = false;
                }
            }
            printf("%08llux ", (unsigned long long)addr);
            for (i = 0; i < 8; ++i)
                printf(" %02x", ((unsigned char *)buf)[by + i]);
            printf(" ");
            for (i = 8; i < 16; ++i)
                printf(" %02x", ((unsigned char *)buf)[by + i]);
            printf("  |");
            for (i = 0; i < 8; ++i)
                printf("%c", buf[by + i] < 32 ? '.' : buf[by + i]);
            for (i = 8; i < 16; ++i)
                printf("%c", buf[by + i] < 32 ? '.' : buf[by + i]);
            printf("|");
            printf("\n");
            by += 16;
            lg -= 16;
            addr += 16;
            if (addr > _.e_range)
                break;
        }

        if (lg != 0)
            memmove(buf, &buf[by], lg);
        by = lg;

        if (addr > _.e_range)
            break;
    }
    if (by != 0 && addr < _.e_range) {
        printf("%08llux ", (unsigned long long)addr);
        for (i = 0; i < 8; ++i)
            printf(i < by ? " %02x" : "   ", ((unsigned char*)buf)[i]);
        printf(" ");
        for (i = 8; i < 16; ++i)
            printf(i < by ? " %02x" : "   ", ((unsigned char*)buf)[i]);
        printf("  |");
        for (i = 0; i < MIN(8, by); ++i)
            printf("%c", buf[i] < 32 ? '.' : buf[i]);
        for (i = 8; i < MIN(16, by); ++i)
            printf("%c", buf[i] < 32 ? '.' : buf[i]);
        printf("|");
        printf("\n");
        addr += by;
    }
    return 0;
}

int main(int argc, char **argv)
{
    _.s_range = 0;
    _.e_range = -1;

    arg_parse(argc, argv, hd_parse, NULL, options, usages);
    return arg_names(argc, argv, hd_main, NULL, options);
}
