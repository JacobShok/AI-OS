/*
 * cmd_basename.c - Strip directory from pathname
 */

#include "picobox.h"
#include "utils.h"

// for --help once again
static void usage_basename(void)
{
    fprintf(stderr,
        "Usage: basename NAME [SUFFIX]\n"
        "Print NAME with any leading directory components removed.\n"
        "If specified, also remove a trailing SUFFIX.\n"
        "\n"
        "Examples:\n"
        "  basename /usr/bin/sort       Output 'sort'\n"
        "  basename /usr/bin/sort .exe  Output 'sort' (removes .exe)\n");
}

int cmd_basename(int argc, char **argv)
{
    char *result;
    char *suffix = NULL;

    // simply checks if first arg is help and gives it, also checks if there are no arg and returns
    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        usage_basename();
        return (argc < 2) ? EXIT_ERROR : EXIT_OK;
    }

    /* Get suffix if provided if argc is 3 */
    if (argc > 2) {
        suffix = argv[2];
    }

    /* Extract basename */
    result = get_basename(argv[1]);
    if (!result) {
        // prints error to stderr
        perror("basename");
        return EXIT_ERROR;
    }

    /* Remove suffix if specified and present */
    if (suffix && str_ends_with(result, suffix)) {
        result[strlen(result) - strlen(suffix)] = '\0';
    }

    printf("%s\n", result);
    free(result);
    // should suffix also be freed ?

    return EXIT_OK;
}
