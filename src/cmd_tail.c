/*
 * cmd_tail.c - Output last N lines of files
 */

#include "picobox.h"
#include "utils.h"

#define DEFAULT_LINES 10
#define MAX_LINES 10000

static void usage_tail(void)
{
    fprintf(stderr,
        "Usage: tail [OPTION]... [FILE]...\n"
        "Print the last 10 lines of each FILE to standard output.\n"
        "\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -n NUM      output the last NUM lines\n"
        "  -h, --help  display this help and exit\n");
}

static int tail_file(const char *filename, int num_lines)
{
    FILE *fp;
    char **lines;
    int count = 0;
    int start = 0;
    int i;
    char buffer[8192];
    int using_stdin = 0;

    if (filename == NULL || strcmp(filename, "-") == 0) {
        fp = stdin;
        using_stdin = 1;
    } else {
        fp = fopen(filename, "r");
        if (!fp) {
            perror(filename);
            return EXIT_ERROR;
        }
    }

    /* Allocate circular buffer for lines */
    lines = calloc(num_lines, sizeof(char *));
    if (!lines) {
        perror("tail");
        if (!using_stdin) fclose(fp);
        return EXIT_ERROR;
    }

    /* Read all lines, keeping only the last N */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        free(lines[count % num_lines]);
        lines[count % num_lines] = strdup(buffer);
        count++;
    }

    if (ferror(fp)) {
        perror(filename ? filename : "stdin");
        if (!using_stdin) fclose(fp);
        for (i = 0; i < num_lines; i++) free(lines[i]);
        free(lines);
        return EXIT_ERROR;
    }

    /* Calculate where to start printing */
    if (count < num_lines) {
        start = 0;
        num_lines = count;
    } else {
        start = count % num_lines;
    }

    /* Print the last N lines */
    for (i = 0; i < num_lines; i++) {
        int idx = (start + i) % num_lines;
        if (lines[idx]) {
            printf("%s", lines[idx]);
        }
    }

    /* Cleanup */
    for (i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);

    if (!using_stdin) {
        fclose(fp);
    }

    return EXIT_OK;
}

int cmd_tail(int argc, char **argv)
{
    int opt;
    int num_lines = DEFAULT_LINES;
    int ret = EXIT_OK;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_tail();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "n:h")) != -1) {
        switch (opt) {
            case 'n':
                num_lines = atoi(optarg);
                if (num_lines < 0 || num_lines > MAX_LINES) {
                    fprintf(stderr, "tail: invalid number of lines: '%s'\n", optarg);
                    return EXIT_ERROR;
                }
                break;
            case 'h':
                usage_tail();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_tail();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* If no files, use stdin */
    if (optind >= argc) {
        return tail_file(NULL, num_lines);
    }

    /* Process each file */
    for (int i = optind; i < argc; i++) {
        if (tail_file(argv[i], num_lines) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    return ret;
}
