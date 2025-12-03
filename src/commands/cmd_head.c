/*
 * cmd_head.c - Output first N lines of files (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: head [OPTIONS] [FILE...]
 * Options:
 *   -n, --lines=NUM   Print first NUM lines (default 10)
 *   -h, --help        Display help message
 */

#include <stdio.h>
#include <string.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int head_run(int argc, char **argv);
void head_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *head_help;
static struct arg_int *head_lines;
static struct arg_file *head_files;
static struct arg_end *head_end;
static void *head_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_head_argtable(void)
{
    head_help = arg_lit0("h", "help", "display this help and exit");
    head_lines = arg_int0("n", "lines", "NUM", "print the first NUM lines instead of 10");
    head_files = arg_filen(NULL, NULL, "FILE", 0, 100, "files to process (or stdin if none)");
    head_end = arg_end(20);

    head_argtable[0] = head_help;
    head_argtable[1] = head_lines;
    head_argtable[2] = head_files;
    head_argtable[3] = head_end;
    head_argtable[4] = NULL;
}

/* ===== HELPER FUNCTION ===== */

static int head_file(const char *filename, int num_lines)
{
    FILE *fp;
    char buffer[8192];
    int lines = 0;
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

    while (lines < num_lines && fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
        lines++;
    }

    if (ferror(fp)) {
        perror(filename ? filename : "stdin");
        if (!using_stdin) fclose(fp);
        return EXIT_ERROR;
    }

    if (!using_stdin) {
        fclose(fp);
    }

    return EXIT_OK;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int head_run(int argc, char **argv)
{
    int nerrors;
    int num_lines = 10;  /* default */
    int i;
    int ret = EXIT_OK;
    int multiple_files;

    build_head_argtable();
    nerrors = arg_parse(argc, argv, head_argtable);

    /* Handle --help */
    if (head_help->count > 0) {
        head_print_usage(stdout);
        arg_freetable(head_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, head_end, "head");
        fprintf(stderr, "Try 'head --help' for more information.\n");
        arg_freetable(head_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Get number of lines if specified */
    if (head_lines->count > 0) {
        num_lines = head_lines->ival[0];
        if (num_lines < 0) {
            fprintf(stderr, "head: invalid number of lines: '%d'\n", num_lines);
            arg_freetable(head_argtable, 4);
            return EXIT_ERROR;
        }
    }

    /* If no files specified, read from stdin */
    if (head_files->count == 0) {
        ret = head_file(NULL, num_lines);
        arg_freetable(head_argtable, 4);
        return ret;
    }

    /* Check if we have multiple files (for headers) */
    multiple_files = (head_files->count > 1);

    /* Process each file */
    for (i = 0; i < head_files->count; i++) {
        /* Print header if multiple files */
        if (multiple_files) {
            if (i > 0) {
                printf("\n");
            }
            printf("==> %s <==\n", head_files->filename[i]);
        }

        if (head_file(head_files->filename[i], num_lines) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    arg_freetable(head_argtable, 4);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void head_print_usage(FILE *out)
{
    build_head_argtable();

    fprintf(out, "Usage: head ");
    arg_print_syntax(out, head_argtable, "\n");
    fprintf(out, "Print the first 10 lines of each FILE to standard output.\n");
    fprintf(out, "With more than one FILE, precede each with a header giving the file name.\n");
    fprintf(out, "With no FILE, or when FILE is -, read standard input.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, head_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  head file.txt             Print first 10 lines of file.txt\n");
    fprintf(out, "  head -n 20 file.txt       Print first 20 lines\n");
    fprintf(out, "  head file1 file2          Print first 10 lines of each file\n");

    arg_freetable(head_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_head_spec = {
    .name = "head",
    .summary = "output the first part of files",
    .long_help = "Print the first 10 lines of each FILE to standard output. "
                 "With more than one FILE, precede each with a header giving the file name.",
    .run = head_run,
    .print_usage = head_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_head_command(void)
{
    register_command(&cmd_head_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_head_spec.run(argc, argv);
}
#endif
