#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static const char* base_name(const char* path) {
    const char* slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static void print_entry(const char* path, const struct stat* st) {
    char type;
    if (S_ISDIR(st->st_mode))
        type = 'd';
    else if (S_ISREG(st->st_mode))
        type = '-';
    else
        type = '?';

    char perms[10];
    perms[0] = (st->st_mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (st->st_mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (st->st_mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (st->st_mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (st->st_mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (st->st_mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (st->st_mode & S_IROTH) ? 'r' : '-';
    perms[7] = (st->st_mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (st->st_mode & S_IXOTH) ? 'x' : '-';
    perms[9] = '\0';

    char owner[32], group[32];
    const struct passwd* pw = getpwuid(st->st_uid);
    if (pw)
        snprintf(owner, sizeof(owner), "%s", pw->pw_name);
    else
        snprintf(owner, sizeof(owner), "%lu", (unsigned long)st->st_uid);

    const struct group* gr = getgrgid(st->st_gid);
    if (gr)
        snprintf(group, sizeof(group), "%s", gr->gr_name);
    else
        snprintf(group, sizeof(group), "%lu", (unsigned long)st->st_gid);

    char size_str[24];
    if (S_ISREG(st->st_mode))
        snprintf(size_str, sizeof(size_str), "%lu", (unsigned long)st->st_size);
    else
        size_str[0] = '\0';

    struct tm tm;
    localtime_r(&st->st_mtime, &tm);
    char date[24];
    strftime(date, sizeof(date), "%b %d %H:%M %Y", &tm);

    printf(
        "%c%s  %4lu  %-10s  %-10s  %10s  %s  %s\n", type, perms,
        (unsigned long)st->st_nlink, owner, group, size_str, date,
        base_name(path)
    );
}

int main(int argc, char* argv[]) {
    int failed = 0;
    int start = (argc > 1) ? 1 : 0;
    int end = (argc > 1) ? argc : 1;
    char dot[] = ".";

    for (int i = start; i < end; i++) {
        const char* path = (argc > 1) ? argv[i] : dot;
        struct stat st;

        if (lstat(path, &st) < 0) {
            fprintf(stderr, "%s: %s\n", path, strerror(errno));
            failed = 1;
            continue;
        }

        print_entry(path, &st);
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
