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
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>

opt_t options[] = {
    OPTION('v', "verbose", "explain what is being done"),
    END_OPTION("change file owner.")
};

char* usages[] = {
    "[OWNER] FILE...",
    NULL,
};

char* __program;

#define CHOWN_OP_KEEP_UID 1
#define CHOWN_OP_KEEP_GID 2

struct param {
    bool verbose;
    int oper;
    uid_t uid;
    gid_t gid;
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
    default:
        fprintf(stderr, "Option -%c non recognized.\n" HELP, arg, __program);
        exit(1);
    }
}

int chown_read_param(const char* arg)
{
    return -1;
}

int do_chown(const char* file)
{
    struct stat st;
    if (_.oper != 0) {
        if (stat(file, &st) != 0) {
            fprintf(stderr, "Unable to acces file %s: %s\n", file, strerror(errno));
            return -1;
        }
    }

    if (_.oper & CHOWN_OP_KEEP_UID)
        _.uid = st.st_uid;
    if (_.oper & CHOWN_OP_KEEP_GID)
        _.gid = st.st_gid;

    if (chown(file, _.uid, _.gid) != 0) {
        fprintf(stderr, "Unable to change owner of file %s: %s\n", file, strerror(errno));
        return -1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    int o, n;
    __program = argv[0];
    memset(&_, 0, sizeof(_));
    n = arg_parse(argc, argv, chmod_parse_args, NULL, options);

    if (n == 0) {
        arg_usage(__program, options, usages);
        return -1;
    }

    bool param = false;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        if (!param) {
            if (chown_read_param(argv[o]) != 0) {
                fprintf(stderr, "Unable to parse parameter %s\n", argv[o]);
                return -1;
            }
            param = true;
            continue;
        }

        do_chown(argv[o]);
    }

    return 0;
}
