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
#include <sys/vfs.h>
#include <unistd.h>

opt_t options[] = {
    OPTION('a', "all", "include pseudo, duplicate, inaccessible file systems"),
    // OPTION('B', "block-size", "scale sizes by SIZE before printing them"),
    OPTION('h', "human-readable", "Print size into a human readable format"),
    OPTION('H', "si", "like -h but use powers of 1000 unstead of 1024"),
    // OPTION('i', "inodes", "list inode information instead of block usage"),
    // OPTION('k', NULL, "like --block-size=1K"),
    // OPTION('l', "local", "limit listing to local file systems"),
    // OPTION(0xd3, "no-sync", "do not invoke sync before getting usage info (default)"),
    OPTION('P', "portability", "use the POSIX output format"),
    // OPTION(0xd4, "sync", "invoke sync before getting usage info"),
    // OPTION(0xd5, "total", "elide all entries insignificant to available space, and produce a grand total"),
    // OPTION('t', "type", "limit listing to file systems of type TYPE"),
    OPTION('T', "print-type", "print file system type"),
    // OPTION('x', "exclude-type", "limit listing to file systems not of type TYPE"),
    END_OPTION("Output reapeteadly STRING, or 'y'.")
};

char* usages[] = {
    "[OPTION]",
    NULL,
};

char* __program;

struct param {
    int readable;
    unsigned exponent;
    int fstype;
    int showall;
} _;
void df_parse_args(void* cfg, unsigned char arg)
{
    switch (arg) {
    case 'a':
        _.showall = 1;
        break;
    case 'h':
        _.readable = 1;
        _.exponent = 1024;
        break;
    case 'H':
        _.readable = 1;
        _.exponent = 1000;
        break;
    case 'P':
        break;
    case 'T':
        _.fstype = 1;
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

struct {
    unsigned short type;
    const char* name;
} fs_regs[] = {
    { 0x4565, "NTFS" },
    { 0xee20, "ext2" },
    { 0xfa12, "FAT12" },
    { 0xfa16, "FAT16" },
    { 0xfa32, "FAT32" },
    { 0x1234, "ISO9660" },
};

void df_fstype(unsigned short type, char* buf, int len)
{
    memset(buf, ' ', len);
    buf[len - 1] = '\0';
    for (int i = 0; fs_regs[i].type != 0; ++i) {
        if (fs_regs[i].type == type) {
            memcpy(buf, fs_regs[i].name, strlen(fs_regs[i].name));
            break;
        }
    }
}

int show_mounted_point(const char *path)
{
    char tmp[1024];
    // Get mounted point of path
#ifdef __kora__
    snprintf(tmp, 1024, "%s/...", path); // Kora hack (...)
    readlink(tmp, tmp, 1024);
#else 
    strncpy(tmp, path, 1024);
#endif

    struct statfs fs;
    statfs(tmp, &fs);
    if (fs.f_bfree > fs.f_blocks) {
        fprintf(stderr, "Incoherrent data on %s\n", path);
        return -1;
    }

    unsigned size = fs.f_blocks / 1024 * fs.f_frsize;
    unsigned used = (fs.f_blocks - fs.f_bfree) / 1024 * fs.f_frsize;
    unsigned avail = fs.f_bfree / 1024 * fs.f_frsize;
    unsigned perc = (fs.f_blocks - fs.f_bfree) * 100 / fs.f_blocks;

    char blocks[36];
    if (_.readable == 0)
        snprintf(blocks, 36, "%10d  %10d  %10d", size, used, avail);
    else {
        char* xsi = "KMGTPEYZ";

        unsigned psize = 0, osize = 0;
        while (size > _.exponent) {
            psize = (size % _.exponent) * 100 / _.exponent;
            size = size / _.exponent;
            osize++;
        }

        unsigned pused = 0, oused = 0;
        while (used > _.exponent) {
            pused = (used % _.exponent) * 100 / _.exponent;
            used = used / _.exponent;
            oused++;
        }

        unsigned pavail = 0, oavail = 0;
        while (avail > _.exponent) {
            pavail = (avail % _.exponent) * 100 / _.exponent;
            avail = avail / _.exponent;
            oavail++;
        }

        snprintf(blocks, 36, "%4u.%02u%c  %4u.%02u%c  %4u.%02u%c",
            size, psize, xsi[osize], 
            used, pused, xsi[oused], 
            avail, pavail, xsi[oavail]);
    }

    char fstype[14];
    fstype[0] = '\0';
    if (_.fstype)
        df_fstype(fs.f_type, fstype, 14);

    printf("%-24s  %s%s  %3d%%  %s", tmp, fstype, blocks, perc, tmp);
    return 0;
}

int main(int argc, char** argv)
{
    int o, n = 0;
    __program = argv[0];
    n = arg_parse(argc, argv, df_parse_args, NULL, options);

    if (n == 0) {
        // LIST MOUNTED POINT ?
        return -1;
    }

    if (_.readable == 0 && _.fstype == 0)
        printf("File system                1K-blocks        Used   Available  Use%%  Mounted on\n");
    else if (_.readable == 0)
        printf("File system               File system   1K-blocks        Used   Available  Use%%  Mounted on\n");
    else if (_.fstype == 0)
        printf("File system                1K-blocks Used      Avail.   Use%%  Mounted on\n");
    else
        printf("File system               File system   1K-blocks Used      Avail.   Use%%  Mounted on\n");

    int ret = 0;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        if (show_mounted_point(argv[o]) != 0)
            ret = -1;
    }

    return ret;
}
