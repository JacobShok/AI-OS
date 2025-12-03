/*
 * cmd_env.c - Display environment variables (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: env [OPTIONS]
 *
 * Options:
 *   -h, --help    Display help message
 *
 * Note: This is a simplified implementation that only prints
 * the current environment. Full env supports setting variables
 * and running commands, which could be added in the future.
 */

#include <stdio.h>
#include <stdlib.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* External environment variable array */
extern char **environ;

/* Forward declarations */
int env_run(int argc, char **argv);
void env_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *env_help;
static struct arg_end *env_end;
static void *env_argtable[3];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_env_argtable(void)
{
    env_help = arg_lit0("h", "help", "display this help and exit");
    env_end = arg_end(20);

    env_argtable[0] = env_help;
    env_argtable[1] = env_end;
    env_argtable[2] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int env_run(int argc, char **argv)
{
    int nerrors;
    int i;

    build_env_argtable();
    nerrors = arg_parse(argc, argv, env_argtable);

    /* Handle --help */
    if (env_help->count > 0) {
        env_print_usage(stdout);
        arg_freetable(env_argtable, 2);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, env_end, "env");
        fprintf(stderr, "Try 'env --help' for more information.\n");
        arg_freetable(env_argtable, 2);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Print all environment variables */
    for (i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }

    arg_freetable(env_argtable, 2);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void env_print_usage(FILE *out)
{
    build_env_argtable();

    fprintf(out, "Usage: env ");
    arg_print_syntax(out, env_argtable, "\n");
    fprintf(out, "Print the current environment.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, env_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "This is a simplified implementation that prints all environment\n");
    fprintf(out, "variables. The full env command supports setting variables and\n");
    fprintf(out, "running commands with modified environments.\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  env                      Print all environment variables\n");
    fprintf(out, "  env | grep PATH          Show PATH-related variables\n");

    arg_freetable(env_argtable, 2);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_env_spec = {
    .name = "env",
    .summary = "run a program in a modified environment",
    .long_help = "Print the current environment. This is a simplified implementation "
                 "that displays all environment variables.",
    .run = env_run,
    .print_usage = env_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_env_command(void)
{
    register_command(&cmd_env_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_env_spec.run(argc, argv);
}
#endif
