/*
 * cmd_rm.c - Remove files or directories
 */

#include "picobox.h"
#include "utils.h"
#include <dirent.h>

static void usage_rm(void)
{
    fprintf(stderr,
        "Usage: rm [OPTION]... FILE...\n"
        "Remove (unlink) the FILE(s).\n"
        "\n"
        "Options:\n"
        "  -r, -R      remove directories and their contents recursively\n"
        "  -f          force removal, ignore nonexistent files\n"
        "  -h, --help  display this help and exit\n");
}

static int rm_recursive(const char *path)
{
    DIR *dir;
    struct dirent *entry;
    char filepath[4096];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror(path);
        return EXIT_ERROR;
    }

    if (!S_ISDIR(st.st_mode)) {
        if (unlink(path) != 0) {
            perror(path);
            return EXIT_ERROR;
        }
        return EXIT_OK;
    }

    /* Remove directory contents */
    dir = opendir(path);
    if (!dir) {
        perror(path);
        return EXIT_ERROR;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        rm_recursive(filepath);
    }

    closedir(dir);

    /* Remove the directory itself */
    if (rmdir(path) != 0) {
        perror(path);
        return EXIT_ERROR;
    }

    return EXIT_OK;
}

int cmd_rm(int argc, char **argv)
{
    int opt;
    int recursive = 0;
    int force = 0;
    int ret = EXIT_OK;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_rm();
        return EXIT_OK;
    }

    /* Parse options - both -r and -R are same */
    while ((opt = getopt(argc, argv, "rRfh")) != -1) {
        switch (opt) {
            case 'r':
            case 'R':
                recursive = 1;
                break;
            case 'f':
                force = 1;
                break;
            case 'h':
                usage_rm();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_rm();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Need at least one file */
    if (optind >= argc) {
        fprintf(stderr, "rm: missing operand\n");
        usage_rm();
        return EXIT_ERROR;
    }

    /* Remove each file */
    for (int i = optind; i < argc; i++) {
        if (is_directory(argv[i])) {
            if (!recursive) {
                fprintf(stderr, "rm: '%s' is a directory (use -r)\n", argv[i]);
                ret = EXIT_ERROR;
                continue;
            }
            if (rm_recursive(argv[i]) != EXIT_OK && !force) {
                ret = EXIT_ERROR;
            }
        } else {
            if (unlink(argv[i]) != 0) {
                if (!force) {
                    perror(argv[i]);
                    ret = EXIT_ERROR;
                }
            }
        }
    }

    return ret;
}
