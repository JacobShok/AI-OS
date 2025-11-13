/*
 * cmd_false.c - Return failure status
 */

#include "picobox.h"

static void usage_false(void)
{
    fprintf(stderr,
        "Usage: false\n"
        "Exit with a status code indicating failure.\n");
}

int cmd_false(int argc, char **argv)
{
    /* Handle --help */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_false();
        return EXIT_OK;
    }

    /* false always fails, ignore all arguments */
    (void)argc;
    (void)argv;
    return EXIT_ERROR;
}
