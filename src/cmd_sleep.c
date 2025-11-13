/*
 * cmd_sleep.c - Delay for specified time
 */

#include "picobox.h"
#include <unistd.h>

static void usage_sleep(void)
{
    fprintf(stderr,
        "Usage: sleep NUMBER[SUFFIX]...\n"
        "Pause for NUMBER seconds.  SUFFIX may be 's' for seconds (default),\n"
        "'m' for minutes, 'h' for hours or 'd' for days.\n"
        "\n"
        "Examples:\n"
        "  sleep 10    Pause for 10 seconds\n"
        "  sleep 1.5   Pause for 1.5 seconds\n"
        "  sleep 2m    Pause for 2 minutes\n");
}

int cmd_sleep(int argc, char **argv)
{
    int opt;
    double seconds = 0;
    char *endptr;
    char suffix;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_sleep();
        return EXIT_OK;
    }

    /* Parse options (only -h for help) */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                usage_sleep();
                return EXIT_OK;
            case '?':
                usage_sleep();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Need duration argument */
    if (optind >= argc) {
        fprintf(stderr, "sleep: missing operand\n");
        usage_sleep();
        return EXIT_ERROR;
    }

    /* Parse number */
    seconds = strtod(argv[optind], &endptr);
    if (endptr == argv[optind] || seconds < 0) {
        fprintf(stderr, "sleep: invalid time interval '%s'\n", argv[optind]);
        return EXIT_ERROR;
    }

    /* Parse suffix */
    if (*endptr != '\0') {
        suffix = *endptr;
        switch (suffix) {
            case 's':
                break;
            case 'm':
                seconds *= 60;
                break;
            case 'h':
                seconds *= 3600;
                break;
            case 'd':
                seconds *= 86400;
                break;
            default:
                fprintf(stderr, "sleep: invalid time suffix '%c'\n", suffix);
                return EXIT_ERROR;
        }
    }

    /* Sleep */
    sleep((unsigned int)seconds);

    return EXIT_OK;
}
