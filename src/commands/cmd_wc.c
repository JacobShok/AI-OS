/*
 * cmd_wc.c - Count lines, words, and bytes (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: wc [OPTIONS] [FILE...]
 * Options:
 *   -l, --lines       Print line counts
 *   -w, --words       Print word counts
 *   -c, --bytes       Print byte counts
 *   -h, --help        Display help message
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int wc_run(int argc, char **argv);
void wc_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *wc_help;
static struct arg_lit *wc_lines;
static struct arg_lit *wc_words;
static struct arg_lit *wc_bytes;
static struct arg_file *wc_files;
static struct arg_end *wc_end;
static void *wc_argtable[7];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_wc_argtable(void)
{
    wc_help = arg_lit0("h", "help", "display this help and exit");
    wc_lines = arg_lit0("l", "lines", "print the newline counts");
    wc_words = arg_lit0("w", "words", "print the word counts");
    wc_bytes = arg_lit0("c", "bytes", "print the byte counts");
    wc_files = arg_filen(NULL, NULL, "FILE", 0, 100, "files to process (or stdin if none)");
    wc_end = arg_end(20);

    wc_argtable[0] = wc_help;
    wc_argtable[1] = wc_lines;
    wc_argtable[2] = wc_words;
    wc_argtable[3] = wc_bytes;
    wc_argtable[4] = wc_files;
    wc_argtable[5] = wc_end;
    wc_argtable[6] = NULL;
}

/* ===== HELPER FUNCTION ===== */

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

/* ===== SECTION 3: RUN FUNCTION ===== */

int wc_run(int argc, char **argv)
{
    int nerrors;
    int show_lines, show_words, show_bytes;
    long total_lines = 0, total_words = 0, total_bytes = 0;
    int i;
    int ret = EXIT_OK;

    build_wc_argtable();
    nerrors = arg_parse(argc, argv, wc_argtable);

    /* Handle --help */
    if (wc_help->count > 0) {
        wc_print_usage(stdout);
        arg_freetable(wc_argtable, 6);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, wc_end, "wc");
        fprintf(stderr, "Try 'wc --help' for more information.\n");
        arg_freetable(wc_argtable, 6);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Determine what to show */
    show_lines = (wc_lines->count > 0);
    show_words = (wc_words->count > 0);
    show_bytes = (wc_bytes->count > 0);

    /* If no options specified, show all */
    if (!show_lines && !show_words && !show_bytes) {
        show_lines = 1;
        show_words = 1;
        show_bytes = 1;
    }

    /* If no files specified, read from stdin */
    if (wc_files->count == 0) {
        ret = wc_file(NULL, show_lines, show_words, show_bytes,
                      &total_lines, &total_words, &total_bytes);
    } else {
        /* Process each file */
        for (i = 0; i < wc_files->count; i++) {
            if (wc_file(wc_files->filename[i], show_lines, show_words, show_bytes,
                       &total_lines, &total_words, &total_bytes) != EXIT_OK) {
                ret = EXIT_ERROR;
            }
        }

        /* Print totals if more than one file */
        if (wc_files->count > 1) {
            if (show_lines) printf(" %7ld", total_lines);
            if (show_words) printf(" %7ld", total_words);
            if (show_bytes) printf(" %7ld", total_bytes);
            printf(" total\n");
        }
    }

    arg_freetable(wc_argtable, 6);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void wc_print_usage(FILE *out)
{
    build_wc_argtable();

    fprintf(out, "Usage: wc ");
    arg_print_syntax(out, wc_argtable, "\n");
    fprintf(out, "Print newline, word, and byte counts for each FILE.\n");
    fprintf(out, "With no FILE, or when FILE is -, read standard input.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, wc_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  wc file.txt               Count lines, words, bytes in file.txt\n");
    fprintf(out, "  wc -l file.txt            Count only lines\n");
    fprintf(out, "  wc -w file1 file2         Count only words in two files\n");

    arg_freetable(wc_argtable, 6);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_wc_spec = {
    .name = "wc",
    .summary = "print newline, word, and byte counts for each file",
    .long_help = "Print newline, word, and byte counts for each FILE, "
                 "and a total line if more than one FILE is specified.",
    .run = wc_run,
    .print_usage = wc_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_wc_command(void)
{
    register_command(&cmd_wc_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_wc_spec.run(argc, argv);
}
#endif
