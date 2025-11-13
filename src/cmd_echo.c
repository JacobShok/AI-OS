/*
 * cmd_echo.c - Print arguments to stdout
 *
 * Usage: echo [OPTIONS] [STRING...]
 * Options:
 *   -n    Do not output trailing newline
 *   -h    Display help message
 */

#include "picobox.h"
#include "utils.h"

static void usage_echo(void)
{
    fprintf(stderr,
        "Usage: echo [OPTION]... [STRING]...\n"
        "Echo the STRING(s) to standard output.\n"
        "\n"
        "Options:\n"
        "  -n              do not output the trailing newline\n"
        "  -h, --help      display this help and exit\n"
        "\n"
        "Examples:\n"
        "  echo hello world        Output 'hello world' followed by newline\n"
        "  echo -n \"no newline\"    Output text without trailing newline\n");
}

int cmd_echo(int argc, char **argv)
{
    int opt;
    int print_newline = 1;  /* By default, print newline */

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_echo();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "nh")) != -1) {
        switch (opt) {
            case 'n':
                print_newline = 0;
                break;
            case 'h':
                usage_echo();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_echo();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Print each argument separated by spaces */
    for (int i = optind; i < argc; i++) {
        if (i > optind) {
            printf(" ");  /* Space between arguments */
        }
        printf("%s", argv[i]);
    }

    /* Print newline unless -n was specified */
    if (print_newline) {
        printf("\n");
    }

    /* Flush output to ensure it appears immediately */
    fflush(stdout);

    return EXIT_OK;
}
