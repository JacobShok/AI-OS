/*
 * cmd_true.c - Return success status (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: true [OPTIONS]
 * Options:
 *   -h, --help    Display help message
 *
 * The true command always returns success (exit code 0),
 * regardless of arguments. It's used in shell scripts for
 * loops and conditionals.
 */

#include <stdio.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int true_run(int argc, char **argv);
void true_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *true_help;
static struct arg_end *true_end;
static void *true_argtable[3];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_true_argtable(void)
{
    true_help = arg_lit0("h", "help", "display this help and exit");
    true_end = arg_end(20);

    true_argtable[0] = true_help;
    true_argtable[1] = true_end;
    true_argtable[2] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int true_run(int argc, char **argv)
{
    int nerrors;

    build_true_argtable();
    nerrors = arg_parse(argc, argv, true_argtable);

    /* Handle --help */
    if (true_help->count > 0) {
        true_print_usage(stdout);
        arg_freetable(true_argtable, 2);
        return EXIT_OK;
    }

    /* Handle parsing errors (though true ignores most arguments) */
    if (nerrors > 0) {
        /* Even with errors, true still succeeds! */
        arg_freetable(true_argtable, 2);
        return EXIT_OK;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */
    /* true always succeeds, nothing to do */

    arg_freetable(true_argtable, 2);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void true_print_usage(FILE *out)
{
    build_true_argtable();

    fprintf(out, "Usage: true ");
    arg_print_syntax(out, true_argtable, "\n");
    fprintf(out, "Exit with a status code indicating success.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, true_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "The true utility always returns with exit code 0 (success).\n");
    fprintf(out, "It is commonly used in shell scripts for infinite loops:\n");
    fprintf(out, "  while true; do\n");
    fprintf(out, "    # commands\n");
    fprintf(out, "  done\n");

    arg_freetable(true_argtable, 2);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_true_spec = {
    .name = "true",
    .summary = "do nothing, successfully",
    .long_help = "Exit with a status code indicating success. "
                 "The true utility always returns 0 (success).",
    .run = true_run,
    .print_usage = true_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_true_command(void)
{
    register_command(&cmd_true_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_true_spec.run(argc, argv);
}
#endif
