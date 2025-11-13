/*
 * cmd_mv.c - Move or rename files
 */

#include "picobox.h"
#include "utils.h"

static void usage_mv(void)
{
    fprintf(stderr,
        "Usage: mv [OPTION]... SOURCE DEST\n"
        "Rename SOURCE to DEST, or move SOURCE to DIRECTORY.\n"
        "\n"
        "Options:\n"
        "  -f          force overwrite\n"
        "  -h, --help  display this help and exit\n");
}

int cmd_mv(int argc, char **argv)
{
    int opt;
    char *src, *dest;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_mv();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "fh")) != -1) {
        switch (opt) {
            case 'f':
                /* Force flag - currently just accepted, not used */
                break;
            case 'h':
                usage_mv();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_mv();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Need source and dest */
    if (optind + 2 != argc) {
        fprintf(stderr, "mv: missing file operand\n");
        usage_mv();
        return EXIT_ERROR;
    }

    src = argv[optind];
    dest = argv[optind + 1];

    /* Try rename first (works on same filesystem) */
    if (rename(src, dest) != 0) {
        perror("mv");
        return EXIT_ERROR;
    }

    return EXIT_OK;
}
