/*
 * cmd_stat.c - Display file status
 */

#include "picobox.h"
#include "utils.h"
#include <time.h>

static void usage_stat(void)
{
    fprintf(stderr,
        "Usage: stat [OPTION]... FILE...\n"
        "Display file or file system status.\n"
        "\n"
        "Options:\n"
        "  -h, --help  display this help and exit\n");
}

int cmd_stat(int argc, char **argv)
{
    int opt;
    struct stat st;
    int ret = EXIT_OK;
    char time_buf[64];

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_stat();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                usage_stat();
                return EXIT_OK;
            case '?':
                usage_stat();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Need at least one file */
    if (optind >= argc) {
        fprintf(stderr, "stat: missing operand\n");
        usage_stat();
        return EXIT_ERROR;
    }

    /* Process each file */
    for (int i = optind; i < argc; i++) {
        if (stat(argv[i], &st) != 0) {
            perror(argv[i]);
            ret = EXIT_ERROR;
            continue;
        }

        printf("  File: %s\n", argv[i]);
        printf("  Size: %lld\n", (long long)st.st_size);
        printf("Blocks: %lld\n", (long long)st.st_blocks);
        printf("  Mode: %04o\n", st.st_mode & 0777);
        printf("  Uid: %d\n", st.st_uid);
        printf("  Gid: %d\n", st.st_gid);

        format_time(st.st_atime, time_buf, sizeof(time_buf));
        printf("Access: %s\n", time_buf);

        format_time(st.st_mtime, time_buf, sizeof(time_buf));
        printf("Modify: %s\n", time_buf);

        format_time(st.st_ctime, time_buf, sizeof(time_buf));
        printf("Change: %s\n", time_buf);
    }

    return ret;
}
