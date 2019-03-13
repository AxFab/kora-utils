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
#include "utils.h"

opt_t options[] = {
    OPTION('a', "all", "do not ignore entries starting with ."),
    OPTION('A', "almost-all", "do not list implied . and .."),
    OPTION( 0 , "author", "with -l, print the author of each file"),
    OPTION('b', "escape", "print C-style escapes for nongraphic characters"),
    OPTION( 0 , "block-size=SIZE", "scale sizes by SIZE before printing them; e.g., '--block-size=M' prints sizes in units of 1,048,576 bytes; see SIZE format below"),
    OPTION('B', "ignore-backups", "do not list implied entries ending with ~"),
    OPTION('c', NULL, "with -lt: sort by, and show, ctime (time of last modification of file status information); with -l: show ctime and sort by name; otherwise: sort by ctime, newest first"),
    OPTION('C', NULL, "list entries by columns"),
    OPTION( 0 , "color", "colorize the output; WHEN can be 'always' (default if omitted), 'auto', or 'never'; more info below"),
    OPTION('d', "directory", "list directories themselves, not their contents"),
    OPTION('D', "dired", "generate output designed for Emacs' dired mode"),
    OPTION('f', NULL, "do not sort, enable -aU, disable -ls --color"),
    OPTION('F', "classify", "append indicator (one of */=>@|) to entries"),
    OPTION( 0 , "file-type", "likewise, except do not append '*'"),
    OPTION( 0 , "format=WORD", "across -x, commas -m, horizontal -x, long -l, single-column -1, verbose -l, vertical -C"),
    OPTION( 0 , "full-time", "like -l --time-style=full-iso"),
    OPTION('g', NULL, "like -l, but do not list owner"),
    OPTION( 0 , "group-directories-first", "group directories before files; can be augmented with a --sort option, but any use of --sort=none (-U) disables grouping"),
    OPTION('G', "no-group", "in a long listing, don't print group names"),
    OPTION('h', "human-readable", "with -l and/or -s, print human readable sizes (e.g., 1K 234M 2G)"),
    OPTION( 0 , "si", "likewise, but use powers of 1000 not 1024"),
    OPTION('H', "dereference-command-line", "follow symbolic links listed on the command line"),
    OPTION( 0 , "dereference-command-line-symlink-to-dir", "follow each command line symbolic link that points to a directory"),
    OPTION( 0 , "hide=PATTERN", "do not list implied entries matching shell PATTERN (overridden by -a or -A)"),
    OPTION( 0 , "indicator-style=WORD", "append indicator with style WORD to entry names: none (default), slash (-p), file-type (--file-type), classify (-F)"),
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
    OPTION( 0 , "show-control-chars", "show nongraphic characters as-is (the default, unless program is 'ls' and output is a terminal)"),
    OPTION('Q', "quote-name", "enclose entry names in double quotes"),
    OPTION( 0 , "quoting-style=WORD", "use quoting style WORD for entry names: literal, locale, shell, shell-always, shell-escape, shell-escape-always, c, escape"),
    OPTION('r', "reverse", "reverse order while sorting"),
    OPTION('R', "recursive", "list subdirectories recursively"),
    OPTION('s', "size", "print the allocated size of each file, in blocks"),
    OPTION('S', NULL, "sort by file size, largest first"),
    OPTION( 0 , "sort=WORD", "sort by WORD instead of name: none (-U), size (-S), time (-t), version (-v), extension (-X)"),
    OPTION( 0 , "time=WORD", "with -l, show time as WORD instead of default modification time: atime or access or use (-u); ctime or status (-c); also use specified time as sort key if --sort=time (newest first)"),
    OPTION( 0 , "time-style=STYLE", "with -l, show times using style STYLE: full-iso, long-iso, iso, locale, or +FORMAT; FORMAT is interpreted like in 'date'; if FORMAT is FORMAT1<newline>FORMAT2, then FORMAT1 applies to non-recent files and FORMAT2 to recent files; if STYLE is prefixed with 'posix-', STYLE takes effect only outside the POSIX locale"),
    OPTION('t', NULL, "sort by modification time, newest first"),
    OPTION('T', "tabsize=COLS", "assume tab stops at each COLS instead of 8"),
    OPTION('u', NULL, "with -lt: sort by, and show, access time; with -l: show access time and sort by name; otherwise: sort by access time, newest first"),
    OPTION('U', NULL, "do not sort; list entries in directory order"),
    OPTION('v', NULL, "natural sort of (version) numbers within text"),
    OPTION('w', "width=COLS", "set output width to COLS.  0 means no limit"),
    OPTION('x', NULL, "list entries by lines instead of by columns"),
    OPTION('X', NULL, "sort alphabetically by entry extension"),
    OPTION('Z', "context", "print any security context of each file"),
    OPTION('1', NULL, "list one file per line.  Avoid '\n' with -q or -b"),
    END_OPTION("List information about the FILEs (the current directory by default).\nSort entries alphabetically if none of -cftuvSUX nor --sort is specified.")
};
char *usages[] = {
    "[OPTION]... [FILE]...",
    NULL,
};

char *__program;

void do_ls(const char *path, void *params)
{
}

void ls_args(void *params, char opt)
{
    switch (opt) {
    case OPT_HELP :
        arg_usage(__program, options, usages) ;
        return 0;
    case OPT_VERS :
        arg_version(__program);
        return 0;
    default:
        fprintf(stderr, "Option -%c non recognized.\n" HELP, *arg, __program);
        return 1;
    }
}

int main(int argc, char **argv)
{
    __program = argv[0];
    int n = arg_parse(argc, argv, ls_args, NULL, options);
    if (n == 0) {
        do_ls("", NULL);
    } else {
        int o;
        for (o = 1; o < argc; ++i) {
            if (argv[o][0] == '-')
                continue;
            do_ls(argv[o], NULL);
        }
    }
    return 0;
}
