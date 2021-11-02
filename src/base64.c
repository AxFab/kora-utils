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

#define BUF_SZ 512

opt_t options[] = {
    OPTION_A('w', "wrap=COLS", "Wrap encoded lines after COLS character (default 76). Use 0 to disable line wrapping."),
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

struct {
    int width;
    int decode;
    int ignore;
    const char *digits;
    int state;
    int value;
    int col;
} _;

static void push_char(int ch)
{
    fputc(ch, stdout);
    if (_.width != 0 && (++_.col % _.width) == 0)
        fputc('\n', stdout);
}

static void transform_char(unsigned char cin)
{
    if (_.state == 0) {
        push_char(_.digits[cin >> 2]);
        _.value = (cin & 3) << 4;
        _.state++;
    } else if (_.state == 1) {
        push_char(_.digits[_.value | cin >> 4]);
        _.value = (cin & 0xF) << 2;
        _.state++;
    } else {
        push_char(_.digits[_.value | cin >> 6]);
        push_char(_.digits[cin & 0x3F]);
        _.value = 0;
        _.state = 0;
    }
}

static void transform_finish()
{
    if (_.state != 0) {
        push_char(_.digits[_.value]);
        push_char('=');
        if (_.state == 1)
            push_char('=');
    }
    _.value = 0;
    _.state = 0;
}

int decode_file(int fd)
{
    return -1;
}

int encode_file(int fd)
{
    int i;
    char buf[BUF_SZ];
    for (;;) {
        int lg = read(fd, buf, BUF_SZ);
        if (lg == 0)
            break;
        if (lg < 0)
            return 1;
        for (i = 0; i < lg; ++i)
            transform_char(buf[i]);
    }
    transform_finish();
    fputc('\n', stdout);
    return 0;
}

void base64_parse_args(void *param, int opt, char *arg)
{
    switch (opt) {
    case 'w':
        _.width = strtol(arg, NULL, 0);
        break;
    case 'd':
        _.decode = 1;
        break;
    case 'i':
        _.ignore = 1;
        break;
    case 'u':
        _.digits = digits64U;
        break;
    }
}

int do_base64(void *param, char *path)
{
    int fd = strcmp(path, "-") ? open(path, O_RDONLY) : 0;
    if (fd == -1) {
        fprintf(stderr, "Unable to open file %s\n", path);
        return 1;
    }

    return _.decode == 0 ? encode_file(fd) : decode_file(fd);
}

int main(int argc, char **argv)
{
    memset(&_, 0, sizeof(_));
    _.digits = digits64;
    _.width = 76;

    int n = arg_parse(argc, argv, base64_parse_args, NULL, options, usages);
    if (n != 0)
        return arg_names(argc, argv, do_base64, NULL, options);
    return _.decode == 0 ? encode_file(0) : decode_file(0);
}
