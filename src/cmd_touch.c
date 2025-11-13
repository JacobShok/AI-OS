/*
 * cmd_touch.c - Create empty files or update timestamps
 *
 * Usage: touch [OPTIONS] FILE...
 * Options:
 *   -c    Do not create file if it doesn't exist
 */

#include "picobox.h"
#include "utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>

static void usage_touch(void)
{
    fprintf(stderr,
        "Usage: touch [OPTION]... FILE...\n"
        "Update the access and modification times of each FILE to the current time.\n"
        "\n"
        "A FILE argument that does not exist is created empty, unless -c is supplied.\n"
        "\n"
        "Options:\n"
        "  -c              do not create any files\n"
        "  -h, --help      display this help and exit\n"
        "\n"
        "Examples:\n"
        "  touch file.txt         Create file.txt or update its timestamp\n"
        "  touch -c existing.txt  Update timestamp only if file exists\n");
}

/*
 * Touch a single file - create if doesn't exist, update timestamp if it does
 */
static int touch_file(const char *filename, int no_create)
{
    int fd;
    struct stat st;

    /* Check if file exists */
    if (stat(filename, &st) == 0) {
        /* File exists - update its timestamp */ // honestly dont know what this means lol
        if (utime(filename, NULL) != 0) {
            perror(filename);
            return EXIT_ERROR;
        }
        return EXIT_OK;
    }

    /* File doesn't exist */
    if (no_create) {
        /* -c flag: don't create, just silently succeed */
        return EXIT_OK;
    }

    /* Create the file */
    fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, 0666);
    if (fd < 0) {
        perror(filename);
        return EXIT_ERROR;
    }

    close(fd);
    return EXIT_OK;
}

int cmd_touch(int argc, char **argv)
{
    int no_create = 0;
    int i;
    int ret = EXIT_OK;

    /* Parse options */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage_touch();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-c") == 0) {
            no_create = 1;
        } else if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else {
            fprintf(stderr, "touch: invalid option '%s'\n", argv[i]);
            usage_touch();
            return EXIT_ERROR;
        }
    }

    /* Check if we have files to touch */
    if (i >= argc) {
        fprintf(stderr, "touch: missing file operand\n");
        usage_touch();
        return EXIT_ERROR;
    }

    /* Touch each file */
    for (; i < argc; i++) {
        if (touch_file(argv[i], no_create) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    return ret;
}
