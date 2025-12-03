/*
 * cmd_cat.c - Concatenate files and print to stdout (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: cat [OPTIONS] [FILE...]
 * Options:
 *   -n, --number      Number all output lines
 *   -h, --help        Display help message
 */

#include <stdio.h>
#include <string.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int cat_run(int argc, char **argv);
void cat_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *cat_help;
static struct arg_lit *cat_number;
static struct arg_file *cat_files;
static struct arg_end *cat_end;
static void *cat_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_cat_argtable(void)
{
    cat_help = arg_lit0("h", "help", "display this help and exit");
    cat_number = arg_lit0("n", "number", "number all output lines");
    cat_files = arg_filen(NULL, NULL, "FILE", 0, 100, "files to concatenate (or stdin if none)");
    cat_end = arg_end(20);

    cat_argtable[0] = cat_help;
    cat_argtable[1] = cat_number;
    cat_argtable[2] = cat_files;
    cat_argtable[3] = cat_end;
    cat_argtable[4] = NULL;
}

/* ===== HELPER FUNCTION ===== */

/*
 * Cat a single file to stdout
 * If filename is "-" or NULL, read from stdin
 */
static int cat_file(const char *filename, int number_lines, int *line_number)
{
    FILE *fp;
    char buffer[8192];
    int using_stdin = 0;

    /* Determine input source */
    if (filename == NULL || strcmp(filename, "-") == 0) {
        fp = stdin;
        using_stdin = 1;
    } else {
        fp = fopen(filename, "r");
        if (fp == NULL) {
            perror(filename);
            return EXIT_ERROR;
        }
    }

    /* Read and output the file */
    if (number_lines) {
        /* Line-by-line with numbering */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%6d  %s", (*line_number)++, buffer);
        }
    } else {
        /* Efficient block read */
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {
                perror("cat: write error");
                if (!using_stdin) fclose(fp);
                return EXIT_ERROR;
            }
        }
    }

    /* Check for read errors */
    if (ferror(fp)) {
        perror(filename ? filename : "stdin");
        if (!using_stdin) fclose(fp);
        return EXIT_ERROR;
    }

    /* Close file if it's not stdin */
    if (!using_stdin) {
        fclose(fp);
    }

    return EXIT_OK;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int cat_run(int argc, char **argv)
{
    int nerrors;
    int number_lines;
    int line_number = 1;
    int i;
    int ret = EXIT_OK;

    build_cat_argtable();
    nerrors = arg_parse(argc, argv, cat_argtable);

    /* Handle --help */
    if (cat_help->count > 0) {
        cat_print_usage(stdout);
        arg_freetable(cat_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, cat_end, "cat");
        fprintf(stderr, "Try 'cat --help' for more information.\n");
        arg_freetable(cat_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    number_lines = (cat_number->count > 0);

    /* If no files specified, read from stdin */
    if (cat_files->count == 0) {
        ret = cat_file(NULL, number_lines, &line_number);
    } else {
        /* Process each file */
        for (i = 0; i < cat_files->count; i++) {
            if (cat_file(cat_files->filename[i], number_lines, &line_number) != EXIT_OK) {
                ret = EXIT_ERROR;
                /* Continue processing remaining files */
            }
        }
    }

    arg_freetable(cat_argtable, 4);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void cat_print_usage(FILE *out)
{
    build_cat_argtable();

    fprintf(out, "Usage: cat ");
    arg_print_syntax(out, cat_argtable, "\n");
    fprintf(out, "Concatenate FILE(s) to standard output.\n");
    fprintf(out, "With no FILE, or when FILE is -, read standard input.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, cat_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  cat file.txt              Output contents of file.txt\n");
    fprintf(out, "  cat file1 file2           Concatenate files and output\n");
    fprintf(out, "  cat -n file.txt           Number all output lines\n");
    fprintf(out, "  cat                       Copy stdin to stdout\n");

    arg_freetable(cat_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_cat_spec = {
    .name = "cat",
    .summary = "concatenate files and print on the standard output",
    .long_help = "Concatenate FILE(s), or standard input, to standard output. "
                 "With -n, number all output lines.",
    .run = cat_run,
    .print_usage = cat_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_cat_command(void)
{
    register_command(&cmd_cat_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_cat_spec.run(argc, argv);
}
#endif
