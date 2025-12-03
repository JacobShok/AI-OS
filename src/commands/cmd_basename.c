/*
 * cmd_basename.c - Strip directory from pathname (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: basename NAME [SUFFIX]
 *   NAME    - Path to process
 *   SUFFIX  - Optional suffix to remove
 *
 * Options:
 *   -h, --help    Display help message
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int basename_run(int argc, char **argv);
void basename_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *basename_help;
static struct arg_str *basename_name;
static struct arg_str *basename_suffix;
static struct arg_end *basename_end;
static void *basename_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_basename_argtable(void)
{
    basename_help = arg_lit0("h", "help", "display this help and exit");
    basename_name = arg_str1(NULL, NULL, "NAME", "pathname to strip directory from");
    basename_suffix = arg_str0(NULL, NULL, "SUFFIX", "optional suffix to remove");
    basename_end = arg_end(20);

    basename_argtable[0] = basename_help;
    basename_argtable[1] = basename_name;
    basename_argtable[2] = basename_suffix;
    basename_argtable[3] = basename_end;
    basename_argtable[4] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int basename_run(int argc, char **argv)
{
    int nerrors;
    char *result;
    const char *suffix;

    build_basename_argtable();
    nerrors = arg_parse(argc, argv, basename_argtable);

    /* Handle --help */
    if (basename_help->count > 0) {
        basename_print_usage(stdout);
        arg_freetable(basename_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, basename_end, "basename");
        fprintf(stderr, "Try 'basename --help' for more information.\n");
        arg_freetable(basename_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Extract basename using helper function */
    result = get_basename(basename_name->sval[0]);
    if (!result) {
        perror("basename");
        arg_freetable(basename_argtable, 4);
        return EXIT_ERROR;
    }

    /* Remove suffix if specified and present */
    if (basename_suffix->count > 0) {
        suffix = basename_suffix->sval[0];
        if (str_ends_with(result, suffix)) {
            result[strlen(result) - strlen(suffix)] = '\0';
        }
    }

    printf("%s\n", result);
    free(result);

    arg_freetable(basename_argtable, 4);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void basename_print_usage(FILE *out)
{
    build_basename_argtable();

    fprintf(out, "Usage: basename ");
    arg_print_syntax(out, basename_argtable, "\n");
    fprintf(out, "Print NAME with any leading directory components removed.\n");
    fprintf(out, "If specified, also remove a trailing SUFFIX.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, basename_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  basename /usr/bin/sort          Output: sort\n");
    fprintf(out, "  basename include/stdio.h .h     Output: stdio\n");
    fprintf(out, "  basename /home/user/file.txt    Output: file.txt\n");

    arg_freetable(basename_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_basename_spec = {
    .name = "basename",
    .summary = "strip directory and suffix from filenames",
    .long_help = "Print NAME with any leading directory components removed. "
                 "If SUFFIX is specified and present, also remove it.",
    .run = basename_run,
    .print_usage = basename_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_basename_command(void)
{
    register_command(&cmd_basename_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_basename_spec.run(argc, argv);
}
#endif
