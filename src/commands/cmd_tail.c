/*
 * cmd_tail.c - Output last N lines of files (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: tail [OPTIONS] [FILE...]
 * Options:
 *   -n, --lines=NUM   Print last NUM lines (default 10)
 *   -h, --help        Display help message
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

#define DEFAULT_LINES 10
#define MAX_LINES 10000

/* Forward declarations */
int tail_run(int argc, char **argv);
void tail_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *tail_help;
static struct arg_int *tail_lines;
static struct arg_file *tail_files;
static struct arg_end *tail_end;
static void *tail_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_tail_argtable(void)
{
    tail_help = arg_lit0("h", "help", "display this help and exit");
    tail_lines = arg_int0("n", "lines", "NUM", "output the last NUM lines instead of 10");
    tail_files = arg_filen(NULL, NULL, "FILE", 0, 100, "files to process (or stdin if none)");
    tail_end = arg_end(20);

    tail_argtable[0] = tail_help;
    tail_argtable[1] = tail_lines;
    tail_argtable[2] = tail_files;
    tail_argtable[3] = tail_end;
    tail_argtable[4] = NULL;
}

/* ===== HELPER FUNCTION ===== */

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

/* ===== SECTION 3: RUN FUNCTION ===== */

int tail_run(int argc, char **argv)
{
    int nerrors;
    int num_lines = DEFAULT_LINES;
    int i;
    int ret = EXIT_OK;
    int multiple_files;

    build_tail_argtable();
    nerrors = arg_parse(argc, argv, tail_argtable);

    /* Handle --help */
    if (tail_help->count > 0) {
        tail_print_usage(stdout);
        arg_freetable(tail_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, tail_end, "tail");
        fprintf(stderr, "Try 'tail --help' for more information.\n");
        arg_freetable(tail_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Get number of lines if specified */
    if (tail_lines->count > 0) {
        num_lines = tail_lines->ival[0];
        if (num_lines < 0 || num_lines > MAX_LINES) {
            fprintf(stderr, "tail: invalid number of lines: '%d'\n", num_lines);
            arg_freetable(tail_argtable, 4);
            return EXIT_ERROR;
        }
    }

    /* If no files specified, read from stdin */
    if (tail_files->count == 0) {
        ret = tail_file(NULL, num_lines);
        arg_freetable(tail_argtable, 4);
        return ret;
    }

    /* Check if we have multiple files (for headers) */
    multiple_files = (tail_files->count > 1);

    /* Process each file */
    for (i = 0; i < tail_files->count; i++) {
        /* Print header if multiple files */
        if (multiple_files) {
            if (i > 0) {
                printf("\n");
            }
            printf("==> %s <==\n", tail_files->filename[i]);
        }

        if (tail_file(tail_files->filename[i], num_lines) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    arg_freetable(tail_argtable, 4);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void tail_print_usage(FILE *out)
{
    build_tail_argtable();

    fprintf(out, "Usage: tail ");
    arg_print_syntax(out, tail_argtable, "\n");
    fprintf(out, "Print the last 10 lines of each FILE to standard output.\n");
    fprintf(out, "With more than one FILE, precede each with a header giving the file name.\n");
    fprintf(out, "With no FILE, or when FILE is -, read standard input.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, tail_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  tail file.txt             Print last 10 lines of file.txt\n");
    fprintf(out, "  tail -n 20 file.txt       Print last 20 lines\n");
    fprintf(out, "  tail file1 file2          Print last 10 lines of each file\n");

    arg_freetable(tail_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_tail_spec = {
    .name = "tail",
    .summary = "output the last part of files",
    .long_help = "Print the last 10 lines of each FILE to standard output. "
                 "With more than one FILE, precede each with a header giving the file name.",
    .run = tail_run,
    .print_usage = tail_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_tail_command(void)
{
    register_command(&cmd_tail_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_tail_spec.run(argc, argv);
}
#endif
