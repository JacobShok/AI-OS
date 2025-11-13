/*
 * cmd_wc.c - Count lines, words, and bytes
 */

#include "picobox.h"
#include "utils.h"
#include <ctype.h>

static void usage_wc(void)
{
    fprintf(stderr,
        "Usage: wc [OPTION]... [FILE]...\n"
        "Print newline, word, and byte counts for each FILE.\n"
        "\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -l      print the newline counts\n"
        "  -w      print the word counts\n"
        "  -c      print the byte counts\n"
        "  -h, --help  display this help and exit\n");
}

static int wc_file(const char *filename, int show_lines, int show_words, int show_bytes,
                   long *total_lines, long *total_words, long *total_bytes)
{
    FILE *fp;
    int ch;
    long lines = 0;
    long words = 0;
    long bytes = 0;
    int in_word = 0;
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

    while ((ch = fgetc(fp)) != EOF) {
        bytes++;

        if (ch == '\n') {
            lines++;
        }

        if (isspace(ch)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }

    if (ferror(fp)) {
        perror(filename ? filename : "stdin");
        if (!using_stdin) fclose(fp);
        return EXIT_ERROR;
    }

    /* Print results */
    if (show_lines) printf(" %7ld", lines);
    if (show_words) printf(" %7ld", words);
    if (show_bytes) printf(" %7ld", bytes);
    if (filename && strcmp(filename, "-") != 0) {
        printf(" %s", filename);
    }
    printf("\n");

    /* Update totals */
    *total_lines += lines;
    *total_words += words;
    *total_bytes += bytes;

    if (!using_stdin) {
        fclose(fp);
    }

    return EXIT_OK;
}

int cmd_wc(int argc, char **argv)
{
    int opt;
    int show_lines = 0;
    int show_words = 0;
    int show_bytes = 0;
    int ret = EXIT_OK;
    int file_count = 0;
    long total_lines = 0;
    long total_words = 0;
    long total_bytes = 0;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_wc();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "lwch")) != -1) {
        switch (opt) {
            case 'l':
                show_lines = 1;
                break;
            case 'w':
                show_words = 1;
                break;
            case 'c':
                show_bytes = 1;
                break;
            case 'h':
                usage_wc();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_wc();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* If no flags specified, show all */
    if (!show_lines && !show_words && !show_bytes) {
        show_lines = show_words = show_bytes = 1;
    }

    /* If no files, use stdin */
    if (optind >= argc) {
        return wc_file(NULL, show_lines, show_words, show_bytes,
                       &total_lines, &total_words, &total_bytes);
    }

    /* Process each file */
    for (int i = optind; i < argc; i++) {
        if (wc_file(argv[i], show_lines, show_words, show_bytes,
                    &total_lines, &total_words, &total_bytes) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
        file_count++;
    }

    /* Print totals if multiple files */
    if (file_count > 1) {
        if (show_lines) printf(" %7ld", total_lines);
        if (show_words) printf(" %7ld", total_words);
        if (show_bytes) printf(" %7ld", total_bytes);
        printf(" total\n");
    }

    return ret;
}
