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
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include "utils.h"

int do_copy(const char* dst, const char* src);

opt_t options[] = {
    OPTION('a', "archive", "same as -dR --preserve=all"),
    OPTION('b', "backup", "make a backup of each existing destination file"),
    // OPTION('d', NULL, "same as --no-dereference and preserve links"),
    OPTION('f', "force", " if an existing destination file cannot be opened, remove it and try again(this option is ignored when the -n option is also used)"),
    OPTION('l', "link", "hard link files instead of copying"),
    // OPTION('L', "dereference", "always follow symbolic links in SOURCE"),
    OPTION('n', "no-clobber ", "do not overwrite an existing file"),
    // OPTION('P', "no-dereference", "never follow symbolic links in SOURCE"),
    // OPTION('p', NULL, "preserve mode, ownership, and timestamps"),
    OPTION('R', NULL, "alias for -r"),
    OPTION('r', "recursive", "copy directories recursively"),
    // OPTION('u', "update", "copy only when the SOURCE file is newer than the destination file or when the destination file is missing"),
    OPTION('v', "verbose", "explain what is being done"),
    END_OPTION("Copy SOURCE to DEST, or multiple SOURCE(s) to DIRECTORY.")
};

char* usages[] = {
    // "[OPTION]... [-T] SOURCE DEST",
    "[OPTION]... SOURCE DEST",
    "[OPTION]... SOURCE... DIRECTORY",
    // "[OPTION]... -t DIRECTORY SOURCE...",
    NULL,
};

char* __program;

#define CHUNK_SIZE 4096

struct copy_info {
    int follow_symlink;
    int is_recursive;
    int do_backup;
    int do_force;
    int dereference;
    int do_link;
    int no_clobber;
    int preserve;
    int update_only;
    int verbose;
    int dst_is_directory;
} _;

#define CP_PR_MODE 1
#define CP_PR_OWNER 2
#define CP_PR_TIMES 4
#define CP_PR_ALL 7


void cp_parse_args(void* params, unsigned char opt)
{
    switch (opt) {
    case 'a':
        _.dereference = -1;
        _.is_recursive = 1;
        _.preserve = CP_PR_ALL;
        break;
    case 'b':
        _.do_backup = 1;
        break;
    case 'd':
        _.dereference = -1;
        break;
    case 'f':
        _.do_force = 1;
        break;
    case 'l':
        _.do_link = 1;
        break;
    case 'L':
        _.dereference = 1;
        break;
    case 'n':
        _.no_clobber = 1;
        break;
    case 'P':
        _.dereference = 0;
        break;
    case 'p':
        _.preserve = CP_PR_ALL;
        break;
    case 'R':
    case 'r':
        _.is_recursive = 1;
        break;
    case 'u':
        _.update_only = 1;
        break;
    case 'v':
        _.verbose = 1;
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


void do_metadata(const char* dst, struct stat* st_src)
{
    chown(dst, st_src->st_uid, st_src->st_gid);
    if (_.preserve & CP_PR_TIMES) {
        struct timeval vtimes[2];
        vtimes[0].tv_sec = st_src->st_atime;
        vtimes[0].tv_usec = 0;
        vtimes[1].tv_sec = st_src->st_mtime;
        vtimes[1].tv_usec = 0;
        utimes(dst, vtimes);
    }
}

int do_copy_link(const char* dst, const char* src, struct stat *st_src)
{
    char buf[CHUNK_SIZE];
    readlink(src, buf, CHUNK_SIZE);
    symlink(buf, dst);
    chmod(dst, st_src->st_mode & 07777);
    do_metadata(dst, st_src);
    return 0;
}

int do_copy_data(int dst_fd, int src_fd, const char* dst, const char* src)
{
    size_t length = lseek(src_fd, 0, SEEK_END);
    lseek(src_fd, 0, SEEK_SET);
    ftruncate(dst_fd, length);

    char buf[CHUNK_SIZE];
    while (length > 0) {
        size_t cap = length < CHUNK_SIZE ? length : CHUNK_SIZE;
        size_t rd = read(src_fd, buf, cap);
        if (rd == 0) {
            fprintf(stderr, "Readind error during copy: %s\n", src);
            return -1;
        }
        size_t wr = write(dst_fd, buf, rd);
        if (wr != rd) {
            fprintf(stderr, "Writing error during copy: %s\n", dst);
            return -1;
        }
        length -= rd;
    }
    return 0;
}

int do_copy_file(const char* dst, const char* src, struct stat* st_src)
{
    struct stat st_dst;
    if (_.do_backup) {
        char buf2[CHUNK_SIZE];
        snprintf(buf2, CHUNK_SIZE, "%s.bkp", dst);
        rename(dst, buf2);
    }
    if (_.no_clobber) {
        int ret = stat(dst, &st_dst);
        if (ret == 0)
            return 0;
    }
    int dst_fd = open(dst, O_WRONLY | O_CREAT, st_src->st_mode & 07777);
    if (dst_fd == -1) {
        if (_.do_force) {
            unlink(dst);
            dst_fd = open(dst, O_WRONLY | O_CREAT, st_src->st_mode & 07777);
        }
        if (dst_fd == -1) {
            fprintf(stderr, "Unable to create destination file %s.\n", dst);
            return -1;
        }
    }
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "Unable to open source file %s.\n", src);
        goto early_fail;
    }

    if (do_copy_data(dst_fd, src_fd, dst, src) != 0)
        goto fail;

    close(src_fd);
    close(dst_fd);
    do_metadata(dst, st_src);
    return 0;
fail:
    close(src_fd);
early_fail:
    close(dst_fd);
    remove(dst);
    return -1;
}

int do_copy_dir(const char* dst, const char* src, struct stat* st_src)
{
    DIR* dir = opendir(src);
    if (dir == NULL) {
        fprintf(stderr, "Failed to open directory %s\n", src);
        return -1;
    }

    if (strcmp(dst, "/") == 0)
        dst = "";
    else
        mkdir(dst, st_src->st_mode & 07777);

    int ret = 0;
    struct dirent* ent = readdir(dir);
    for (; ent != NULL; ent = readdir(dir)) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char buf[CHUNK_SIZE];
        char buf2[CHUNK_SIZE];
        snprintf(buf, CHUNK_SIZE, "%s/%s", src, ent->d_name);
        snprintf(buf2, CHUNK_SIZE, "%s/%s", dst, ent->d_name);
        ret |= do_copy(buf2, buf);
    }

    closedir(dir);
    do_metadata(dst, st_src);
    return ret;
}


int do_copy(const char* dst, const char* src)
{
    int ret;
    struct stat st_src;
    if (_.follow_symlink)
        ret = lstat(src, &st_src);
    else
        ret = stat(src, &st_src);

    if (ret != 0) {
        fprintf(stderr, "Unable to access file %s: %s\n", src, strerror(errno));
        return ret;
    }

    if (S_ISLNK(st_src.st_mode))
        return do_copy_link(dst, src, &st_src);

    if (S_ISDIR(st_src.st_mode)) {
        if (!_.is_recursive) {
            fprintf(stderr, "Omitting directory %s\n", src);
            return -1;
        }
        return do_copy_dir(dst, src, &st_src);
    }

    if (S_ISREG(st_src.st_mode))
        return do_copy_file(dst, src, &st_src);

    fprintf(stderr, "Unable to copy devices %s\n", src);
    return -1;
}

int main(int argc, char **argv) 
{
    int o;
    __program = argv[0];
    memset(&_, 0, sizeof(_));

    int n = arg_parse(argc, argv, (parsa_t)cp_parse_args, &_, options);
    if (n < 2) {
        fprintf(stderr, "Missing operand.\n");
        arg_usage(__program, options, usages);
        return -1;
    } else if (n > 2) {
        _.dst_is_directory = 1;
    }

    char* destination = NULL;
    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        destination = argv[o];
    }

    for (o = 1; o < argc; ++o) {
        if (argv[o][0] == '-')
            continue;
        if (destination == argv[o])
            break;
        if (do_copy(destination, argv[o]) != 0)
            return -1;
    }

    return 0;
}
