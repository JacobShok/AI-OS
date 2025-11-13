/*
 * cmd_chmod.c - Change file permissions
 */

#include "picobox.h"
#include "utils.h"

static void usage_chmod(void)
{
    fprintf(stderr,
        "Usage: chmod MODE FILE...\n"
        "Change the mode of each FILE to MODE.\n"
        "\n"
        "MODE is an octal number like 755 or 644.\n"
        "\n"
        "Examples:\n"
        "  chmod 755 script.sh   Make file rwxr-xr-x\n"
        "  chmod 644 file.txt    Make file rw-r--r--\n");
}

int cmd_chmod(int argc, char **argv)
{
    int opt;
    mode_t mode;
    char *endptr;
    long val;
    int ret = EXIT_OK;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_chmod();
        return EXIT_OK;
    }

    /* Parse options (currently only -h for help) */
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                usage_chmod();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_chmod();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* MODE is first non-option argument */
    if (optind >= argc) {
        fprintf(stderr, "chmod: missing mode operand\n");
        usage_chmod();
        return EXIT_ERROR;
    }

    /* Parse mode (octal) */
    val = strtol(argv[optind], &endptr, 8);
    if (*endptr != '\0' || val < 0 || val > 0777) {
        fprintf(stderr, "chmod: invalid mode '%s'\n", argv[optind]);
        return EXIT_ERROR;
    }
    mode = (mode_t)val;
    optind++;  /* Move past MODE argument */

    /* Need at least one file */
    if (optind >= argc) {
        fprintf(stderr, "chmod: missing file operand\n");
        usage_chmod();
        return EXIT_ERROR;
    }

    /* Change mode for each file */
    for (int i = optind; i < argc; i++) {
        if (chmod(argv[i], mode) != 0) {
            perror(argv[i]);
            ret = EXIT_ERROR;
        }
    }

    return ret;
}
