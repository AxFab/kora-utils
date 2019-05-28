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
#include <kora/mcrs.h>
#include <kora/llist.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "utils.h"

opt_t options[] = {
    OPTION('a', "all", "do not ignore entries starting with ."),
    OPTION('A', "almost-all", "do not list implied . and .."),
    OPTION(0x80, "author", "with -l, print the author of each file"),
    OPTION('b', "escape", "print C-style escapes for nongraphic characters"),
    OPTION(0x81, "block-size=SIZE", "scale sizes by SIZE before printing them; e.g., '--block-size=M' prints sizes in units of 1,048,576 bytes; see SIZE format below"),
    OPTION('B', "ignore-backups", "do not list implied entries ending with ~"),
    OPTION('c', NULL, "with -lt: sort by, and show, ctime (time of last modification of file status information); with -l: show ctime and sort by name; otherwise: sort by ctime, newest first"),
    OPTION('C', NULL, "list entries by columns"),
    OPTION(0x82, "color", "colorize the output; WHEN can be 'always' (default if omitted), 'auto', or 'never'; more info below"),
    OPTION('d', "directory", "list directories themselves, not their contents"),
    OPTION('D', "dired", "generate output designed for Emacs' dired mode"),
    OPTION('f', NULL, "do not sort, enable -aU, disable -ls --color"),
    OPTION('F', "classify", "append indicator (one of */=>@|) to entries"),
    OPTION(0x83, "file-type", "likewise, except do not append '*'"),
    OPTION(0x84, "format=WORD", "across -x, commas -m, horizontal -x, long -l, single-column -1, verbose -l, vertical -C"),
    OPTION(0x85, "full-time", "like -l --time-style=full-iso"),
    OPTION('g', NULL, "like -l, but do not list owner"),
    OPTION(0x86, "group-directories-first", "group directories before files; can be augmented with a --sort option, but any use of --sort=none (-U) disables grouping"),
    OPTION('G', "no-group", "in a long listing, don't print group names"),
    OPTION('h', "human-readable", "with -l and/or -s, print human readable sizes (e.g., 1K 234M 2G)"),
    OPTION(0x87, "si", "likewise, but use powers of 1000 not 1024"),
    OPTION('H', "dereference-command-line", "follow symbolic links listed on the command line"),
    OPTION(0x88, "dereference-command-line-symlink-to-dir", "follow each command line symbolic link that points to a directory"),
    OPTION(0x89, "hide=PATTERN", "do not list implied entries matching shell PATTERN (overridden by -a or -A)"),
    OPTION(0x8a, "indicator-style=WORD", "append indicator with style WORD to entry names: none (default), slash (-p), file-type (--file-type), classify (-F)"),
    OPTION('i', "inode", "print the index number of each file"),
    OPTION('I', "ignore=PATTERN", "do not list implied entries matching shell PATTERN"),
    OPTION('k', "kibibytes", "default to 1024-byte blocks for disk usage"),
    OPTION('l', NULL, "use a long listing format"),
    OPTION('L', "dereference", "when showing file information for a symbolic link, show information for the file the link references rather than for the link itself"),
    OPTION('m', NULL, "fill width with a comma separated list of entries"),
    OPTION('n', "numeric-uid-gid", "like -l, but list numeric user and group IDs"),
    OPTION('N', "literal", "print entry names without quoting"),
    OPTION('o', NULL, "like -l, but do not list group information"),
    OPTION('p', "indicator-style=slash", "append / indicator to directories"),
    OPTION('q', "hide-control-chars", "print ? instead of nongraphic characters"),
    OPTION(0x8b, "show-control-chars", "show nongraphic characters as-is (the default, unless program is 'ls' and output is a terminal)"),
    OPTION('Q', "quote-name", "enclose entry names in double quotes"),
    OPTION(0x8c, "quoting-style=WORD", "use quoting style WORD for entry names: literal, locale, shell, shell-always, shell-escape, shell-escape-always, c, escape"),
    OPTION('r', "reverse", "reverse order while sorting"),
    OPTION('R', "recursive", "list subdirectories recursively"),
    OPTION('s', "size", "print the allocated size of each file, in blocks"),
    OPTION('S', NULL, "sort by file size, largest first"),
    OPTION(0x8d, "sort=WORD", "sort by WORD instead of name: none (-U), size (-S), time (-t), version (-v), extension (-X)"),
    OPTION(0x8e, "time=WORD", "with -l, show time as WORD instead of default modification time: atime or access or use (-u); ctime or status (-c); also use specified time as sort key if --sort=time (newest first)"),
    OPTION(0x8f, "time-style=STYLE", "with -l, show times using style STYLE: full-iso, long-iso, iso, locale, or +FORMAT; FORMAT is interpreted like in 'date'; if FORMAT is FORMAT1<newline>FORMAT2, then FORMAT1 applies to non-recent files and FORMAT2 to recent files; if STYLE is prefixed with 'posix-', STYLE takes effect only outside the POSIX locale"),
    OPTION('t', NULL, "sort by modification time, newest first"),
    OPTION('T', "tabsize=COLS", "assume tab stops at each COLS instead of 8"),
    OPTION('u', NULL, "with -lt: sort by, and show, access time; with -l: show access time and sort by name; otherwise: sort by access time, newest first"),
    OPTION('U', NULL, "do not sort; list entries in directory order"),
    OPTION('v', NULL, "natural sort of (version) numbers within text"),
    OPTION('w', "width=COLS", "set output width to COLS.  0 means no limit"),
    OPTION('x', NULL, "list entries by lines instead of by columns"),
    OPTION('X', NULL, "sort alphabetically by entry extension"),
    OPTION('Z', "context", "print any security context of each file"),
    OPTION('1', NULL, "list one file per line.  Avoid '\\n' with -q or -b"),
    END_OPTION("List information about the FILEs (the current directory by default).\nSort entries alphabetically if none of -cftuvSUX nor --sort is specified.")
};
char *usages[] = {
    "[OPTION]... [FILE]...",
    NULL,
};

char *__program;

#define LS_SHOW_HIDE 1
#define LS_SHOW_DOTS 2
#define LS_SHOW_BCKP 4

#define LS_CTIME 1
#define MIN_COLUMN_WIDTH 3

struct ls_dir {
    struct dirent dent;
    struct stat info;
    char lpath[256];
    llnode_t node;
};

struct ls_params {
    int format;
    int flags;
    int columns;
    int sort;
    int sort_time;
    bool newline;
    bool escape;
    bool by_columns;
    bool summary;
    bool one_by_line;
    bool use_color;
    int width;

    int **column_width;
    int *column_fwidth;

    llhead_t list;
    struct ls_dir **table;
};

void ls_parse_args(struct ls_params *params, unsigned char opt)
{
    switch (opt) {
    case 'a' :
        params->flags |= LS_SHOW_HIDE | LS_SHOW_DOTS;
        break;
    case 'A' :
        params->flags |= LS_SHOW_HIDE;
        break;
    case 'B' :
        params->flags &= ~LS_SHOW_BCKP;
        break;
    case 'c':
        // Time = ctime
        break;
    case 'C':
        params->format = 0;
        break;
    case 'l':
        params->format = 1;
        break;
    case 'g':
        // No owner !
        params->format = 1;
        break;

    case OPT_HELP: // --help
        arg_usage(__program, options, usages);
        exit(0);
    case OPT_VERS: // --version
        arg_version(__program);
        exit(0);
    default:
        fprintf(stderr, "Option -%c non recognized.\n" HELP, opt, __program);
        exit(1);
    }
}

int ls_read_dir(const char *path, struct ls_params *params)
{
    struct dirent *de;
    struct ls_dir *di;
    char fpath[4096];
    DIR *ctx = opendir(path);
    if (ctx == NULL) {
        fprintf(stderr, "ls: cannot access '%s': No such file or directory\n", path);
        return -1;
    }

    memset(&params->list, 0, sizeof(params->list));
    while ((de = readdir(ctx)) != NULL) {
        if (!(params->flags & LS_SHOW_DOTS)) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                continue;
        }

        if (!(params->flags & LS_SHOW_HIDE) && de->d_name[0] == '.')
            continue;
        if (!(params->flags & LS_SHOW_BCKP) && de->d_name[strlen(de->d_name) - 1] == '~')
            continue;
        di = malloc(sizeof(struct ls_dir));
        memset(&di->node, 0, sizeof(di->node));
        memcpy(&di->dent, de, sizeof(di->dent));
        strncpy(fpath, path, 4096);
        strncat(fpath, "/", 4096);
        strncat(fpath, de->d_name, 4096);
        int ret = lstat(fpath, &di->info);
        if (ret < 0) {
            fprintf(stderr, "ls: cannot access '%s': No such file or directory\n", fpath);
            return -1;
        }
        if (de->d_type == 10)
            readlink(fpath, di->lpath, 256);
        // if possible display ASAP
        ll_append(&params->list, &di->node);
    }
    return 0;
}

void ls_table(struct ls_params *params)
{
    struct ls_dir *di;
    params->table = calloc(params->list.count_, sizeof(struct ls_dir *));
    int fileno = 0;
    for ll_each(&params->list, di, struct ls_dir, node)
        params->table[fileno++] = di;
}

int ls_count_columns(struct ls_params *params)
{
    int i, max_cols = MIN(MAX(0, params->width / MIN_COLUMN_WIDTH), params->list.count_ + 1) + 1;

    /* Initialize column length buffers */
    params->column_width = calloc(max_cols, sizeof(int *));
    params->column_fwidth = calloc(max_cols, sizeof(int));
    for (i = 1; i < max_cols; ++i)
        params->column_width[i] = calloc(i, sizeof(int));

    /* compute row length for each column counts */
    int fileno;
    struct ls_dir *di;
    for (fileno = 0; fileno < params->list.count_; ++fileno) {
        di = params->table[fileno] ;
        int len = strlen(di->dent.d_name) + 2;
        for (i = 1; i < max_cols; ++i) {
            if (params->column_fwidth[i] >= params->width + 2)
                continue;
            int idx = fileno / ((params->list.count_ + i - 1) / i);
            if (params->column_width[i][idx] < len) {
                params->column_fwidth[i] += len - params->column_width[i][idx];
                params->column_width[i][idx] = len;
            }
        }
    }

    /* Find maximum allowed column */
    while (max_cols-- > 0) {
        if (params->column_fwidth[i] >= params->width + 2)
            continue;
        int cols = MAX(1, max_cols - 1);
        int rows = (params->list.count_ + cols - 1) / cols;
        int spot = cols * rows - params->list.count_;
        if (spot < rows)
            return cols;
    }
    return 1;
}

static void ls_color(int mode, int attr)
{
    switch (mode) {
    case 1: // p
        fputs("\033[40;33m", stdout);
        break;
    case 2: // c
    case 6: // b
        fputs("\033[93m", stdout);
        break;
    case 4: // d
        fputs("\033[94m", stdout);
        break;
    case 10: // l
        fputs("\033[96m", stdout);
        break;
    case 12: // s
        fputs("\033[95m", stdout);
        break;
    case 8:
        if (attr & 0100)
            fputs("\033[92m", stdout);
        else
            fputs("\033[0m", stdout);
        break;
    default:
        fputs("\033[45m", stdout);
        break;
    }
}

void ls_display_many_per_line(const char *path, struct ls_params *params)
{
    ls_table(params);
    int cols = ls_count_columns(params);

    if (params->newline)
        fputc('\n', stdout);
    if (params->summary) {
        fputs(path, stdout);
        fputs(":\n", stdout);
    }

    struct ls_dir *di;
    int i, l, rows = (params->list.count_ + cols - 1) / cols;
    for (l = 0; l < rows; l++) {
        for (i = l; i < params->list.count_; i += rows) {
            di = params->table[i];
            int col = i / rows;
            int attr = di->info.st_mode & 0xFFF;
            int mode = di->info.st_mode >> 12;
            if (params->use_color)
                ls_color(mode, attr);
            fputs(di->dent.d_name, stdout);
            if (params->use_color)
                fputs("\033[0m", stdout);
            if (col == cols - 1 || (col == cols - 2 && l > MAX(1, params->list.count_ % rows)))
                fputc('\n', stdout);
            else {
                int sp = params->column_width[cols][col] - strlen(di->dent.d_name);
                while (sp-- > 0)
                    fputc(' ', stdout);
            }
        }
    }

    params->newline = true;
    // TODO -- Free
}

const char *mode_txt = "0pc3d5b7-9lBsDEF";

const char *rights_txts[] = {
    "---", "--x", "-w-", "-wx",
    "r--", "r-x", "rw-", "rwx",
};

void ls_display_one_per_line(const char *path, struct ls_params *params)
{
    if (params->newline)
        fputc('\n', stdout);
    if (params->summary) {
        fputs(path, stdout);
        fputs(":\n", stdout);
    }

    struct ls_dir *di;
    for ll_each(&params->list, di, struct ls_dir, node) {

        int i, l;
        int attr = di->info.st_mode & 0xFFF;
        int mode = di->info.st_mode >> 12;

        fputc(mode_txt[mode & 15], stdout);
        l = attr;
        for (i = 0; i < 3; ++i) {
            fputs(rights_txts[(l >> 6) & 7], stdout);
            l = l << 3;
        }

        fputs(" ", stdout);

        // Inode
        printf("%*d ", 6, (int)di->info.st_ino);
        // User
        printf("%*s ", 6, "Fabien");
        // Size
        printf("%*d%s ", 5, (int)di->info.st_size, "");
        // Time
        char tmbuf[24];
        struct tm tmstc;
        localtime_r(&di->info.st_ctime, &tmstc);
        strftime(tmbuf, 24, "%b %d %H:%M ", &tmstc);
        fputs(tmbuf, stdout);

        if (params->use_color)
            ls_color(mode, attr);
        fputs(di->dent.d_name, stdout);
        if (params->use_color)
            fputs("\033[0m", stdout);

        if (mode == 10) {
            fputs(" -> ", stdout);
            fputs(di->lpath, stdout);
        }

        fputc('\n', stdout);
    }

    params->newline = true;
    // TODO -- Free
}

int do_ls(const char *path, struct ls_params *params)
{
    if (ls_read_dir(path, params) != 0)
        return -1;
    // sort
    if (params->format == 0)
        ls_display_many_per_line(path, params);
    else
        ls_display_one_per_line(path, params);
    return 0;
}


int main(int argc, char **argv)
{
    __program = argv[0];
    struct ls_params params;
    params.format = 0;
    params.flags = LS_SHOW_BCKP;
    params.summary = false;
    params.newline = false;
    params.use_color = true;
    params.width = tty_cols();

    int dirs = arg_parse(argc, argv, (parsa_t)ls_parse_args, &params, options);

    if (dirs == 0)
        do_ls(".", &params);
    else {
        int o;
        params.summary = true;
        for (o = 1; o < argc; ++o) {
            if (argv[o][0] == '-')
                continue;
            do_ls(argv[o], &params);
        }
    }
    return 0;
}
