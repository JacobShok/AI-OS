/*
 * cmd_grep.c - Search for patterns in files (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: grep [OPTIONS] PATTERN [FILE...]
 * Options:
 *   -i, --ignore-case   Ignore case distinctions
 *   -n, --line-number   Print line numbers
 *   -v, --invert-match  Invert match (select non-matching lines)
 *   -h, --help          Display help message
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int grep_run(int argc, char **argv);
void grep_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *grep_help;
static struct arg_lit *grep_ignore_case;
static struct arg_lit *grep_line_numbers;
static struct arg_lit *grep_invert;
static struct arg_str *grep_pattern;
static struct arg_file *grep_files;
static struct arg_end *grep_end;
static void *grep_argtable[8];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_grep_argtable(void)
{
    grep_help = arg_lit0("h", "help", "display this help and exit");
    grep_ignore_case = arg_lit0("i", "ignore-case", "ignore case distinctions");
    grep_line_numbers = arg_lit0("n", "line-number", "print line numbers");
    grep_invert = arg_lit0("v", "invert-match", "invert match (select non-matching lines)");
    grep_pattern = arg_str1(NULL, NULL, "PATTERN", "pattern to search for");
    grep_files = arg_filen(NULL, NULL, "FILE", 0, 100, "files to search (or stdin if none)");
    grep_end = arg_end(20);

    grep_argtable[0] = grep_help;
    grep_argtable[1] = grep_ignore_case;
    grep_argtable[2] = grep_line_numbers;
    grep_argtable[3] = grep_invert;
    grep_argtable[4] = grep_pattern;
    grep_argtable[5] = grep_files;
    grep_argtable[6] = grep_end;
    grep_argtable[7] = NULL;
}

/* ===== HELPER FUNCTION ===== */

static int grep_file(const char *filename, const char *pattern, int ignore_case, int line_numbers, int invert)
{
    FILE *fp;
    char buffer[8192];
    int line_num = 0;
    int found = 0;
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

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        line_num++;
        int match;

        if (ignore_case) {
            match = (strcasestr(buffer, pattern) != NULL);
        } else {
            match = (strstr(buffer, pattern) != NULL);
        }

        if (invert) {
            match = !match;
        }

        if (match) {
            if (line_numbers) {
                printf("%d:", line_num);
            }
            printf("%s", buffer);
            found = 1;
        }
    }

    if (!using_stdin) {
        fclose(fp);
    }

    return found ? EXIT_OK : EXIT_ERROR;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int grep_run(int argc, char **argv)
{
    int nerrors;
    int ignore_case = 0;
    int line_numbers = 0;
    int invert = 0;
    const char *pattern;
    int i;
    int ret = EXIT_OK;

    build_grep_argtable();
    nerrors = arg_parse(argc, argv, grep_argtable);

    /* Handle --help */
    if (grep_help->count > 0) {
        grep_print_usage(stdout);
        arg_freetable(grep_argtable, 7);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, grep_end, "grep");
        fprintf(stderr, "Try 'grep --help' for more information.\n");
        arg_freetable(grep_argtable, 7);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check flags */
    if (grep_ignore_case->count > 0) {
        ignore_case = 1;
    }
    if (grep_line_numbers->count > 0) {
        line_numbers = 1;
    }
    if (grep_invert->count > 0) {
        invert = 1;
    }

    /* Get pattern */
    pattern = grep_pattern->sval[0];

    /* If no files, use stdin */
    if (grep_files->count == 0) {
        ret = grep_file(NULL, pattern, ignore_case, line_numbers, invert);
        arg_freetable(grep_argtable, 7);
        return ret;
    }

    /* Process each file */
    for (i = 0; i < grep_files->count; i++) {
        if (grep_file(grep_files->filename[i], pattern, ignore_case, line_numbers, invert) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    arg_freetable(grep_argtable, 7);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void grep_print_usage(FILE *out)
{
    build_grep_argtable();

    fprintf(out, "Usage: grep ");
    arg_print_syntax(out, grep_argtable, "\n");
    fprintf(out, "Search for PATTERN in each FILE.\n");
    fprintf(out, "With no FILE, or when FILE is -, read standard input.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, grep_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  grep hello file.txt       Search for 'hello' in file.txt\n");
    fprintf(out, "  grep -i hello file.txt    Case-insensitive search\n");
    fprintf(out, "  grep -n hello file.txt    Show line numbers\n");
    fprintf(out, "  grep -v hello file.txt    Show lines NOT matching 'hello'\n");

    arg_freetable(grep_argtable, 7);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_grep_spec = {
    .name = "grep",
    .summary = "search for patterns in files",
    .long_help = "Search for PATTERN in each FILE. With no FILE, or when FILE is -, read standard input.",
    .run = grep_run,
    .print_usage = grep_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_grep_command(void)
{
    register_command(&cmd_grep_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_grep_spec.run(argc, argv);
}
#endif
