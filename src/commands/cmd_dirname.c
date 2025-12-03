/*
 * cmd_dirname.c - Extract directory from pathname (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: dirname NAME
 *   NAME    - Path to extract directory from
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
int dirname_run(int argc, char **argv);
void dirname_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *dirname_help;
static struct arg_str *dirname_name;
static struct arg_end *dirname_end;
static void *dirname_argtable[4];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_dirname_argtable(void)
{
    dirname_help = arg_lit0("h", "help", "display this help and exit");
    dirname_name = arg_str1(NULL, NULL, "NAME", "pathname to extract directory from");
    dirname_end = arg_end(20);

    dirname_argtable[0] = dirname_help;
    dirname_argtable[1] = dirname_name;
    dirname_argtable[2] = dirname_end;
    dirname_argtable[3] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int dirname_run(int argc, char **argv)
{
    int nerrors;
    char *result;

    build_dirname_argtable();
    nerrors = arg_parse(argc, argv, dirname_argtable);

    /* Handle --help */
    if (dirname_help->count > 0) {
        dirname_print_usage(stdout);
        arg_freetable(dirname_argtable, 3);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, dirname_end, "dirname");
        fprintf(stderr, "Try 'dirname --help' for more information.\n");
        arg_freetable(dirname_argtable, 3);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Extract dirname using helper function */
    result = get_dirname(dirname_name->sval[0]);
    if (!result) {
        perror("dirname");
        arg_freetable(dirname_argtable, 3);
        return EXIT_ERROR;
    }

    printf("%s\n", result);
    free(result);

    arg_freetable(dirname_argtable, 3);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void dirname_print_usage(FILE *out)
{
    build_dirname_argtable();

    fprintf(out, "Usage: dirname ");
    arg_print_syntax(out, dirname_argtable, "\n");
    fprintf(out, "Output NAME with its last non-slash component and trailing slashes removed.\n");
    fprintf(out, "If NAME contains no slashes, output '.' (current directory).\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, dirname_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  dirname /usr/bin/sort      Output: /usr/bin\n");
    fprintf(out, "  dirname stdio.h            Output: .\n");
    fprintf(out, "  dirname /home/user/        Output: /home\n");

    arg_freetable(dirname_argtable, 3);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_dirname_spec = {
    .name = "dirname",
    .summary = "strip last component from file name",
    .long_help = "Output NAME with its trailing /component removed. "
                 "If NAME contains no '/' characters, output '.'.",
    .run = dirname_run,
    .print_usage = dirname_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_dirname_command(void)
{
    register_command(&cmd_dirname_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_dirname_spec.run(argc, argv);
}
#endif
