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
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

opt_t options[] = {
    OPTION('v', "verbose", "explain what is being done"),
    END_OPTION("Change file mode bits")
};

char* usages[] = {
    "[MODE] FILES...",
    NULL,
};

char* __program;

#define CHMOD_OP_SET 0
#define CHMOD_OP_ADD 1
#define CHMOD_OP_SUB 2

struct param {
    bool verbose;
    int oper;
    int mode;
} _;

void chmod_parse_args(void* cfg, unsigned char arg)
{
    switch (arg) {
    case 'v':
        _.verbose = true;
        break;
    case OPT_HELP: // --help
        arg_usage(__program, options, usages);
        exit(0);
    case OPT_VERS: // --version
        arg_version(__program);
        exit(0);
    }
}

int chmod_read_param(const char* arg)
{
    // Valid: 0xxx | 
    char apply = 'a';
    char oper = '=';
    _.mode = 0;

    // Read if octal code
    if (*arg == '0') {
        char* ptr;
        _.mode = strtoul(arg, &ptr, 8);
        if (*ptr != '\0' || _.mode > 07777)
            return -1;
        return 0;
    }

    // [ugoa]?
    if (*arg == 'u' || *arg == 'g' || *arg == 'o' || *arg == 'a') {
        apply = arg[0];
        arg++;
        if (*arg == '\0')
            return -1;
    }

    // [+-=]?
    if (*arg == '+' || *arg == '-' || *arg == '=') {
        oper = arg[0];
        arg++;
        if (*arg == '\0')
            return -1;
    }

    if (*arg >= '0' && *arg < '8') {
        // [0-7]{1}
        int m = *arg - '0';
        _.mode = m << 6 | m << 3 | m;
        arg++;
    } else {
        // [rwxXst]*
        for (;;arg++) {
            if (*arg == 'r')
                _.mode |= 0444;
            else if (*arg == 'w')
                _.mode |= 0233;
            else if (*arg == 'x')
                _.mode |= 0111;
            else if (*arg == 's')
                _.mode |= S_ISGID | S_ISUID;
            else if (*arg == 't')
                _.mode |= S_ISVTX; // Sticky bit
            else
                break;
        }
    }

    if (*arg != '\0')
        return -1;

    if (oper == '=')
        _.oper = CHMOD_OP_SET;
    else if (oper == '+')
        _.oper = CHMOD_OP_ADD;
    else if (oper == '-')
        _.oper = CHMOD_OP_SUB;

    if (apply == 'u')
        _.mode &= 05700;
    else if (apply == 'g')
        _.mode &= 03070;
    else if (apply == 'o')
        _.mode &= 01007;
    else
        _.mode &= 01777;

    return 0;
}

int do_chmod(const char* file)
{
    int ret;
    struct stat st;
    if (_.oper != CHMOD_OP_SET) {
        if (stat(file, &st) != 0) {
            fprintf(stderr, "Unable to acces file %s: %s\n", file, strerror(errno));
            return -1;
        }
    }

    if (_.oper == CHMOD_OP_SET)
        ret = chmod(file, _.mode);
    else if (_.oper == CHMOD_OP_ADD) 
        ret = chmod(file, (st.st_mode & 07777) | _.mode);
    else if (_.oper == CHMOD_OP_SUB) 
        ret = chmod(file, (st.st_mode & 07777) & ~_.mode);

    if (ret != 0) {
        fprintf(stderr, "Unable to change mode of file %s: %s\n", file, strerror(errno));
        return -1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    int o, n = 0;
    __program = argv[0];
    memset(&_, 0, sizeof(_));
    int n = arg_parse(argc, argv, chmod_parse_args, NULL, options);

    if (n == 0) {
        arg_usage(__program, options, usages);
        return -1;
    }

    bool param = false;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        if (!param) {
            if (chmod_read_param(argv[o]) != 0) {
                fprintf(stderr, "Unable to parse parameter %s\n", argv[o]);
                return -1;
            }
            param = true;
            continue;
        }
        
        do_chmod(argv[o]);
    }

    return 0;
}
