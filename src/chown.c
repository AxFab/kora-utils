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

#define CHOWN_OP_KEEP_UID 1
#define CHOWN_OP_KEEP_GID 2

struct param {
    bool verbose;
    int oper;
    uid_t uid;
    gid_t gid;
    bool param;
} _;

void chown_parse_args(void* cfg, int opt, char *arg)
{
    switch (opt) {
    case 'v':
        _.verbose = true;
        break;
    }
}

int chown_read_param(const char* arg)
{
    return -1;
}

int do_chown(void *cfg, char* file)
{
    if (_.param == false) {
        _.param = true;
        if (chown_read_param(file) == 0)
            return 0;
        fprintf(stderr, "Unable to parse parameter %s\n", file);
        return -1;
    }

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
    memset(&_, 0, sizeof(_));
    int n = arg_parse(argc, argv, chown_parse_args, NULL, options, usages);
    if (n == 0) {
        arg_usage(argv[0], options, usages);
        return -1;
    }
    return arg_names(argc, argv, do_chown, NULL, options);
}
