/*
 * cmd_du.c - Estimate file space usage
 */

#include "picobox.h"
#include "utils.h"
#include <dirent.h>

static void usage_du(void)
{
    fprintf(stderr,
        "Usage: du [OPTION]... [FILE]...\n"
        "Summarize disk usage of each FILE, recursively for directories.\n"
        "\n"
        "Options:\n"
        "  -h          print sizes in human readable format\n"
        "  -s          display only a total for each argument\n"
        "  --help      display this help and exit\n");
}

static off_t du_recursive(const char *path, int summary, int human)
{
    struct stat st;
    DIR *dir;
    struct dirent *entry;
    char filepath[4096];
    off_t total = 0;

    if (lstat(path, &st) != 0) {
        perror(path);
        return 0;
    }

    total = st.st_blocks * 512;  /* Convert blocks to bytes */

    if (S_ISDIR(st.st_mode)) {
        dir = opendir(path);
        if (!dir) {
            perror(path);
            return total;
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
            total += du_recursive(filepath, 1, human);
        }

        closedir(dir);
    }

    if (!summary) {
        char size_buf[32];
        if (human) {
            format_size(total, size_buf, sizeof(size_buf));
            printf("%s\t%s\n", size_buf, path);
        } else {
            printf("%lld\t%s\n", (long long)(total / 1024), path);
        }
    }

    return total;
}

int cmd_du(int argc, char **argv)
{
    int summary = 0;
    int human = 0;
    int i = 1;
    off_t total;
    char size_buf[32];

    /* Parse options */
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "--help") == 0) {
            usage_du();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-s") == 0) {
            summary = 1;
            i++;
        } else if (strcmp(argv[i], "-h") == 0) {
            human = 1;
            i++;
        } else {
            fprintf(stderr, "du: invalid option '%s'\n", argv[i]);
            usage_du();
            return EXIT_ERROR;
        }
    }

    /* If no path, use current directory */
    if (i >= argc) {
        total = du_recursive(".", summary, human);
        if (summary) {
            if (human) {
                format_size(total, size_buf, sizeof(size_buf));
                printf("%s\t.\n", size_buf);
            } else {
                printf("%lld\t.\n", (long long)(total / 1024));
            }
        }
        return EXIT_OK;
    }

    /* Process each path */
    for (; i < argc; i++) {
        total = du_recursive(argv[i], summary, human);
        if (summary) {
            if (human) {
                format_size(total, size_buf, sizeof(size_buf));
                printf("%s\t%s\n", size_buf, argv[i]);
            } else {
                printf("%lld\t%s\n", (long long)(total / 1024), argv[i]);
            }
        }
    }

    return EXIT_OK;
}
