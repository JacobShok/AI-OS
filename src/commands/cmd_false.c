/*
 * cmd_false.c - Return failure status (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: false [OPTIONS]
 * Options:
 *   -h, --help    Display help message
 *
 * The false command always returns failure (exit code 1),
 * regardless of arguments. It's used in shell scripts for
 * testing and conditionals.
 */

#include <stdio.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int false_run(int argc, char **argv);
void false_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *false_help;
static struct arg_end *false_end;
static void *false_argtable[3];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_false_argtable(void)
{
    false_help = arg_lit0("h", "help", "display this help and exit");
    false_end = arg_end(20);

    false_argtable[0] = false_help;
    false_argtable[1] = false_end;
    false_argtable[2] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int false_run(int argc, char **argv)
{
    int nerrors;

    build_false_argtable();
    nerrors = arg_parse(argc, argv, false_argtable);

    /* Handle --help (special case: help succeeds even though it's "false") */
    if (false_help->count > 0) {
        false_print_usage(stdout);
        arg_freetable(false_argtable, 2);
        return EXIT_OK;  /* Help text prints successfully */
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        /* false fails regardless of errors */
        arg_freetable(false_argtable, 2);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */
    /* false always fails */

    arg_freetable(false_argtable, 2);
    return EXIT_ERROR;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void false_print_usage(FILE *out)
{
    build_false_argtable();

    fprintf(out, "Usage: false ");
    arg_print_syntax(out, false_argtable, "\n");
    fprintf(out, "Exit with a status code indicating failure.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, false_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "The false utility always returns with exit code 1 (failure).\n");
    fprintf(out, "It is commonly used in shell scripts for testing:\n");
    fprintf(out, "  if false; then\n");
    fprintf(out, "    echo \"This will never run\"\n");
    fprintf(out, "  fi\n");

    arg_freetable(false_argtable, 2);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_false_spec = {
    .name = "false",
    .summary = "do nothing, unsuccessfully",
    .long_help = "Exit with a status code indicating failure. "
                 "The false utility always returns 1 (failure).",
    .run = false_run,
    .print_usage = false_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_false_command(void)
{
    register_command(&cmd_false_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_false_spec.run(argc, argv);
}
#endif
