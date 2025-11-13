/*
 * cmd_mkdir.c - Create directories
 *
 * Usage: mkdir [OPTIONS] DIRECTORY...
 * Options:
 *   -p    Create parent directories as needed
 *   -m    Set file mode (permissions)
 */

#include "picobox.h"
#include "utils.h"
#include <sys/stat.h>

static void usage_mkdir(void)
{
    fprintf(stderr,
        "Usage: mkdir [OPTION]... DIRECTORY...\n"
        "Create the DIRECTORY(ies), if they do not already exist.\n"
        "\n"
        "Options:\n"
        "  -p              make parent directories as needed\n"
        "  -m MODE         set file mode (as in chmod), not a=rwx - umask\n"
        "  -h, --help      display this help and exit\n"
        "\n"
        "Examples:\n"
        "  mkdir newdir           Create directory 'newdir'\n"
        "  mkdir -p a/b/c         Create nested directories\n"
        "  mkdir -m 755 mydir     Create with specific permissions\n");
}

/*
 * Create a single directory with given mode ./build/picobox mkdir [OPTIONS] <directory>
 */
static int create_single_dir(const char *path, mode_t mode)
{
    if (mkdir(path, mode) != 0) {
        if (errno != EEXIST) {
            perror(path);
            return EXIT_ERROR;
        }
        /* Directory already exists - not an error with -p */
    }
    return EXIT_OK;
}

/*
 * Create directory and all parents as needed
 * Example: "a/b/c" creates "a", then "a/b", then "a/b/c"
 */
static int create_parents(const char *path, mode_t mode)
{
    char *path_copy;
    char *p;
    int ret = EXIT_OK;

    /* Make a copy we can modify */
    path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("mkdir");
        return EXIT_ERROR;
    }

    /* Create each directory in the path */
    for (p = path_copy + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            /* Create this level */
            if (create_single_dir(path_copy, mode) != EXIT_OK) {
                /* Only fail if it's not EEXIST */
                if (errno != EEXIST) {
                    ret = EXIT_ERROR;
                    goto cleanup;
                }
            }

            /* Restore the slash */
            *p = '/';
        }
    }

    /* Create the final directory */
    if (create_single_dir(path_copy, mode) != EXIT_OK) {
        ret = EXIT_ERROR;
    }

cleanup:
    free(path_copy);
    return ret;
}

/*
 * Parse octal mode string (e.g., "755" -> 0755)
 */
static int parse_mode(const char *mode_str, mode_t *mode)
{
    char *endptr;
    long val;

    val = strtol(mode_str, &endptr, 8);  /* Parse as octal */
    if (*endptr != '\0' || val < 0 || val > 0777) {
        fprintf(stderr, "mkdir: invalid mode '%s'\n", mode_str);
        return -1;
    }

    *mode = (mode_t)val;
    return 0;
}

int cmd_mkdir(int argc, char **argv)
{
    int parents = 0;
    mode_t mode = 0777;  /* Default: rwxrwxrwx (modified by umask) */
    int i;
    int ret = EXIT_OK;

    /* Parse options */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage_mkdir();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-p") == 0) {
            parents = 1;
        } else if (strcmp(argv[i], "-m") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "mkdir: option '-m' requires an argument\n");
                usage_mkdir();
                return EXIT_ERROR;
            }
            if (parse_mode(argv[i], &mode) != 0) {
                return EXIT_ERROR;
            }
        } else if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else {
            fprintf(stderr, "mkdir: invalid option '%s'\n", argv[i]);
            usage_mkdir();
            return EXIT_ERROR;
        }
    }

    /* Check if we have directories to create */
    if (i >= argc) {
        fprintf(stderr, "mkdir: missing operand\n");
        usage_mkdir();
        return EXIT_ERROR;
    }

    /* Create each directory */
    for (; i < argc; i++) {
        if (parents) {
            if (create_parents(argv[i], mode) != EXIT_OK) {
                ret = EXIT_ERROR;
            }
        } else {
            if (create_single_dir(argv[i], mode) != EXIT_OK) {
                ret = EXIT_ERROR;
            }
        }
    }

    return ret;
}
