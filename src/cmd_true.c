/*
 * cmd_true.c - Return success status
 */

#include "picobox.h"

static void usage_true(void)
{
    fprintf(stderr,
        "Usage: true\n"
        "Exit with a status code indicating success.\n");
}

int cmd_true(int argc, char **argv)
{
    /* Handle --help */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_true();
        return EXIT_OK;
    }

    /* true always succeeds, ignore all arguments */
    (void)argc;
    (void)argv;
    return EXIT_OK;
}
