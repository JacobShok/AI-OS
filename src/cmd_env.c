/*
 * cmd_env.c - Display environment variables
 */

#include "picobox.h"

extern char **environ;

static void usage_env(void)
{
    fprintf(stderr,
        "Usage: env [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]\n"
        "Set each NAME to VALUE in the environment and run COMMAND.\n"
        "\n"
        "With no COMMAND, print the resulting environment.\n"
        "\n"
        "Options:\n"
        "  -i, --ignore-environment  start with an empty environment\n"
        "  -h, --help                display this help and exit\n");
}

int cmd_env(int argc, char **argv)
{
    int opt;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_env();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "ih")) != -1) {
        switch (opt) {
            case 'i':
                /* Ignore environment - simple implementation doesn't support this */
                fprintf(stderr, "env: -i option not yet implemented\n");
                return EXIT_ERROR;
            case 'h':
                usage_env();
                return EXIT_OK;
            case '?':
                usage_env();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Simple implementation: just print environment */
    /* Full implementation would handle NAME=VALUE and COMMAND */
    for (int i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }

    return EXIT_OK;
}
