/*
 * cmd_df.c - Report filesystem disk space usage
 */

#include "picobox.h"
#include "utils.h"
#include <sys/statvfs.h>

static void usage_df(void)
{
    fprintf(stderr,
        "Usage: df [OPTION]... [FILE]...\n"
        "Show information about the file system on which each FILE resides.\n"
        "\n"
        "Options:\n"
        "  -h          print sizes in human readable format\n"
        "  --help      display this help and exit\n");
}

int cmd_df(int argc, char **argv)
{
    struct statvfs vfs;
    int human = 0;
    int i = 1;
    const char *path = ".";
    char size_buf[32];

    /* Parse options */
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "--help") == 0) {
            usage_df();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-h") == 0) {
            human = 1;
            i++;
        } else {
            fprintf(stderr, "df: invalid option '%s'\n", argv[i]);
            usage_df();
            return EXIT_ERROR;
        }
    }

    // if user provides a file path use it otherwise skip and go defualt
    if (i < argc) {
        path = argv[i];
    }


    /*
    statvfs(): System call that fills a struct statvfs with filesystem info
    Returns information about the filesystem that contains the given path
    If it fails (bad path, permission denied, etc.), print error and exit
    */
    if (statvfs(path, &vfs) != 0) {
        perror("df");
        return EXIT_ERROR;
    }

    printf("Filesystem     ");
    if (human) {
        printf("Size  Used Avail Use%%\n");
    } else {
        printf("1K-blocks      Used Available Use%%\n");
    }

    off_t total = vfs.f_blocks * vfs.f_frsize;
    off_t avail = vfs.f_bavail * vfs.f_frsize;
    off_t used = total - (vfs.f_bfree * vfs.f_frsize);
    int use_percent = total > 0 ? (int)((used * 100) / total) : 0;

    printf("%-15s", path);

    if (human) {
        format_size(total, size_buf, sizeof(size_buf));
        printf("%5s ", size_buf);
        format_size(used, size_buf, sizeof(size_buf));
        printf("%5s ", size_buf);
        format_size(avail, size_buf, sizeof(size_buf));
        printf("%5s ", size_buf);
    } else {
        printf("%10lld %10lld %10lld ",
               (long long)(total / 1024),
               (long long)(used / 1024),
               (long long)(avail / 1024));
    }

    printf("%3d%%\n", use_percent);

    return EXIT_OK;
}
