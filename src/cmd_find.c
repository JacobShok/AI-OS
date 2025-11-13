/*
 * cmd_find.c - Search for files
 */

#include "picobox.h"
#include "utils.h"
#include <dirent.h>
#include <fnmatch.h>

static void usage_find(void)
{
    fprintf(stderr,
        "Usage: find [PATH...] [EXPRESSION]\n"
        "Search for files in a directory hierarchy.\n"
        "\n"
        "Options:\n"
        "  -name PATTERN   base of file name matches PATTERN\n"
        "  -type TYPE      file is of type TYPE (f=file, d=directory)\n"
        "  -h, --help      display this help and exit\n");
}

/*
*.c - ends with .c
cmd_* - starts with cmd_
*test* - contains test
main.c - exact name
*/

static void find_recursive(const char *path, const char *name_pattern, char type_filter)
{
    DIR *dir;
    struct dirent *entry;
    char filepath[4096];
    struct stat st;

    dir = opendir(path);
    if (!dir) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (lstat(filepath, &st) != 0) {
            continue;
        }

        int match = 1;

        /* Check name pattern */
        if (name_pattern && fnmatch(name_pattern, entry->d_name, 0) != 0) {
            match = 0;
        }

        /* Check type filter */
        if (type_filter) {
            if (type_filter == 'f' && !S_ISREG(st.st_mode)) {
                match = 0;
            } else if (type_filter == 'd' && !S_ISDIR(st.st_mode)) {
                match = 0;
            }
        }

        if (match) {
            printf("%s\n", filepath);
        }

        /* Recurse into directories */
        if (S_ISDIR(st.st_mode)) {
            find_recursive(filepath, name_pattern, type_filter);
        }
    }

    closedir(dir);
}

int cmd_find(int argc, char **argv)
{
    int i = 1;
    char *start_path = ".";
    char *name_pattern = NULL;
    char type_filter = 0;

    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage_find();
        return EXIT_OK;
    }

    /* Parse path */
    if (i < argc && argv[i][0] != '-') {
        start_path = argv[i++];
    }

    /* Parse options */
    while (i < argc) {
        if (strcmp(argv[i], "-name") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "find: missing argument to -name\n");
                return EXIT_ERROR;
            }
            name_pattern = argv[i++];
        } else if (strcmp(argv[i], "-type") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "find: missing argument to -type\n");
                return EXIT_ERROR;
            }
            type_filter = argv[i++][0];
        } else {
            fprintf(stderr, "find: unknown option '%s'\n", argv[i]);
            usage_find();
            return EXIT_ERROR;
        }
    }

    find_recursive(start_path, name_pattern, type_filter);
    return EXIT_OK;
}
