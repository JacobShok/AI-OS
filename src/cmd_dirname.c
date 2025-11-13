/*
 * cmd_dirname.c - Extract directory from pathname
 */

#include "picobox.h"
#include "utils.h"

static void usage_dirname(void)
{
    fprintf(stderr,
        "Usage: dirname NAME\n"
        "Output NAME with its last non-slash component and trailing slashes removed.\n"
        "\n"
        "Examples:\n"
        "  dirname /usr/bin/sort   Output '/usr/bin'\n"
        "  dirname stdio.h         Output '.'\n");
}

int cmd_dirname(int argc, char **argv)
{
    int opt;
    char *result;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_dirname();
        return EXIT_OK;
    }

    /* Parse options (only -h for help) */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                usage_dirname();
                return EXIT_OK;
            case '?':
                usage_dirname();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Need exactly one argument (NAME) */
    if (optind >= argc) {
        fprintf(stderr, "dirname: missing operand\n");
        usage_dirname();
        return EXIT_ERROR;
    }

    result = get_dirname(argv[optind]);
    if (!result) {
        perror("dirname");
        return EXIT_ERROR;
    }

    printf("%s\n", result);
    free(result);

    return EXIT_OK;
}
