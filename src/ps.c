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
#include "dirent.h"
#include "kora/llist.h"

opt_t options[] = {

    // -e All process (not only urrent user )
    // -r Only running process (Add column PID - TTY - STAT - TIME - CMD )
    // -C <command> Matching command name
    // -G <group> Matching group name
    // -p <proess id>
    // -U <username | id>
    // -F Extra full format (UID PID PPID C SZ RSS PSR STIME TTY TIME CMD)
    // -f Full format  (UID PID PPID STIME TTY TIME CMD)
    // -H Show process hierachy (No ascii art !?)
    // -m Show threads after process
    END_OPTION("Show list of processes.")
};

char *usages[] = {
    "[OPTION]",
    NULL,
};

typedef struct pstat pstat_t;
struct pstat {
    int pid;
    int ppid;
    int thrds;
    char state;
    llnode_t node;
    char name[64];
    pstat_t *child;
    pstat_t *sibling;
    pstat_t *parent;
};

llhead_t procs;

void ps_push(pstat_t *parent, pstat_t *child)
{
    child->parent = parent;
    child->sibling = parent->child;
    parent->child = child;
}

void ps_tree()
{
    pstat_t *w1;
    pstat_t *w2;
    for ll_each(&procs, w1, pstat_t, node) {
        for ll_each(&procs, w2, pstat_t, node) {
            if (w1->pid == w2->ppid)
                ps_push(w1, w2);

        }
    }
}

void ps_read(int pid)
{
    pstat_t *ps = calloc(1, sizeof(pstat_t));
    ll_append(&procs, &ps->node);
    ps->pid = pid;
    char path[64];
    char tmp[64];
    snprintf(path, 64, "/proc/%d/status", pid);
    FILE *fp = fopen(path, "r");
    while (fgets(path, 64, fp) != NULL) {
        if (memcmp(path, "Name:", 5) == 0) {
            strcpy(ps->name, strchr(path, '\t') + 1);
            strchr(ps->name, '\n')[0] = '\0';
        } else if (memcmp(path, "PPid:", 5) == 0) {
            strcpy(tmp, strchr(path, '\t') + 1);
            ps->ppid = strtol(tmp, NULL, 10);
        } else if (memcmp(path, "Threads:", 7) == 0) {
            strcpy(tmp, strchr(path, '\t') + 1);
            ps->thrds = strtol(tmp, NULL, 10);
        } else if (memcmp(path, "State:", 6) == 0) {
            strcpy(tmp, strchr(path, '\t') + 1);
            ps->state = tmp[0];
        }
    }
    fclose(fp);
}
void strrncat(char *buf, const char *str, int len)
{
    int ds = strlen(str);
    memmove(&buf[ds], buf, len - ds);
    memcpy(buf, str, ds);
}

#define BUF_SZ  128

const char *TBL[] = {
    //" `-- ", " |-- ", " |   ",
    " |__ ", " |__ ", " |   ",
    // " └── ", " ├── ", " │   ",
};


void ps_write(pstat_t *ps)
{
    printf("%5d %5d ", ps->pid, ps->ppid);
    printf(" %c ", ps->state);
    printf("%3d ", ps->thrds);
    char graph[BUF_SZ] = { 0 };
    pstat_t *w = ps;
    while (w->parent) {
        if (w == ps) {
            if (w->sibling == NULL)
                strrncat(graph, TBL[0], BUF_SZ);
            else
                strrncat(graph, TBL[1], BUF_SZ);
        } else {
            if (w->sibling == NULL)
                strrncat(graph, "     ", BUF_SZ);
            else
                strrncat(graph, TBL[2], BUF_SZ);
        }
        w = w->parent;
    }
    printf("%s", graph);
    printf("%s\n", ps->name);

    w = ps->child;
    while (w) {
        ps_write(w);
        w = w->sibling;
    }
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

    llist_init(&procs);
    struct dirent *de;
    char *rent;
    DIR *dir = opendir("/proc");
    while ((de = readdir(dir)) != NULL) {
        int pid = strtol(de->d_name, &rent, 10);
        if (rent[0] != '\0')
            continue;
        ps_read(pid);
    }
    closedir(dir);

    ps_tree();
    // ll_sort(&procs, comparator);

    pstat_t *ps;

    // PID - TTY - TIME - NAME

    printf("PID   PPID  St #Th NAME\n");
    for ll_each(&procs, ps, pstat_t, node) {
        if (ps->parent)
            continue;
        ps_write(ps);
    }

    return 0;
}

