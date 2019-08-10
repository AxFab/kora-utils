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
 *   - - - - - - - - - - - - - - -
 *
 */
#include "utils.h"
#include <fcntl.h>
#include <unistd.h>


#define BUF_SZ 512

opt_t options[] = {
    OPTION('w', "wrap", "Wrap encoded lines after COLS character (default 76). Use 0 to disable line wrapping."),
    OPTION('d', "decode", "Decode data."),
    OPTION('i', "ignore-garbage", "When decoding, ignore non-alphabet characters."),
    OPTION('u', "url-compatible", "Use URL compatible digits."),
    END_OPTION("Base64 encode or decode FILE, or standard input, to standard output.\n"
               "With no FILE, or when FILE is -, read standard input.")
};

char *usages[] = {
    "[OPTION] [FILE]",
    NULL,
};

const char *digits64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
const char *digits64U = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";

const char *digits;
int width = 76;
int state = 0;
int value = 0;
int col = 0;

static void push_char(int ch)
{
    fputc(ch, stdout);
    if (width != 0 && (++col % width) == 0)
        fputc('\n', stdout);
}

static void transform_char(unsigned char cin)
{
    if (state == 0) {
        push_char(digits[cin >> 2]);
        value = (cin & 3) << 4;
        state++;
    } else if (state == 1) {
        push_char(digits[value | cin >> 4]);
        value = (cin & 0xF) << 2;
        state++;
    } else {
        push_char(digits[value | cin >> 6]);
        push_char(digits[cin & 0x3F]);
        value = 0;
        state = 0;
    }
}

static void transform_finish()
{
    if (state != 0) {
        push_char(digits[value]);
        push_char('=');
        if (state == 1)
            push_char('=');
    }
    value = 0;
    state = 0;
}

int main(int argc, char **argv)
{
    int i, j;
    digits = digits64;
    width = 76;
    int oflg = 0 ; // O_RDONLY;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-' || argv[i][0] == '\0')
            continue;
        unsigned char *arg = argv[i][1] == '-' ? arg_long(&argv[i][2], options) : (unsigned char *)&argv[i][1];
        for (; *arg; ++arg) {
            switch (*arg) {
            case 'w' :
                ++i;
                width = strtol(argv[i], NULL, 0);
                break;
            case 'd' :
                break;
            case 'i' :
                break;
            case 'u' :
                digits = digits64U;
                break;
            case OPT_HELP :
                arg_usage(argv[0], options, usages) ;
                return 0;
            case OPT_VERS :
                arg_version(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Option -%c non recognized.\n" HELP, *arg, argv[0]);
                return 1;
            }
        }
    }

    char buf[BUF_SZ] ;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1] != '\0')
            continue;
        char *path = argv[i];
        int fd = strcmp(path, "-") ? open(path, oflg) : 1;
        if (fd == -1) {
            fprintf(stderr, "Unable to open file %s\n", path);
            return 1;
        }

        for (;;) {
            int lg = read(fd, buf, BUF_SZ);
            if (lg == 0)
                break ;
            if (lg < 0)
                return 1;
            for (j = 0; j < lg; ++j)
                transform_char(buf[j]);
        }
        transform_finish();
        fputc('\n', stdout);
    }
    return 0;
}
