/*
 * cmd_echo.c - Print arguments to stdout (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: echo [OPTIONS] [STRING...]
 * Options:
 *   -n, --no-newline    Do not output trailing newline
 *   -h, --help          Display help message
 */

#include <stdio.h>
#include <string.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int echo_run(int argc, char **argv);
void echo_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES (static to this file) ===== */

static struct arg_lit *echo_help;
static struct arg_lit *echo_no_newline;
static struct arg_str *echo_args;
static struct arg_end *echo_end;
static void *echo_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

/**
 * Build the argtable definition for echo command
 *
 * This defines the command-line interface:
 *   -h, --help        Display help (no argument)
 *   -n, --no-newline  Don't print newline (no argument)
 *   STRING...         Zero or more strings to print (positional args)
 */
static void build_echo_argtable(void)
{
    /* Define help flag */
    echo_help = arg_lit0("h", "help", "display this help and exit");

    /* Define no-newline flag */
    echo_no_newline = arg_lit0("n", "no-newline", "do not output trailing newline");

    /* Define string arguments (0 to 100 strings) */
    echo_args = arg_strn(NULL, NULL, "STRING", 0, 100, "strings to print");

    /* Error handling structure (max 20 errors) */
    echo_end = arg_end(20);

    /* Build the argtable array */
    echo_argtable[0] = echo_help;
    echo_argtable[1] = echo_no_newline;
    echo_argtable[2] = echo_args;
    echo_argtable[3] = echo_end;
    echo_argtable[4] = NULL;  /* Always NULL-terminate */
}

/* ===== SECTION 3: RUN FUNCTION (main implementation) ===== */

/**
 * Execute the echo command
 *
 * This is the main implementation of echo.
 * It prints all arguments separated by spaces,
 * optionally followed by a newline.
 *
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return EXIT_OK on success, EXIT_ERROR on failure
 */
int echo_run(int argc, char **argv)
{
    int nerrors;
    int i;

    /* Build argtable definition */
    build_echo_argtable(); // Sets up arg_lit, arg_str structures

    /* Parse command-line arguments */
    nerrors = arg_parse(argc, argv, echo_argtable);

    /* Handle --help FIRST (before checking for errors) */
    if (echo_help->count > 0) {
        echo_print_usage(stdout);
        arg_freetable(echo_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        /* Print errors to stderr */
        arg_print_errors(stderr, echo_end, "echo");
        fprintf(stderr, "Try 'echo --help' for more information.\n");
        arg_freetable(echo_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Print each argument, separated by spaces */
    for (i = 0; i < echo_args->count; i++) {
        /* Add space between arguments (but not before first one) */
        if (i > 0) {
            printf(" ");
        }
        /* Print the argument */
        printf("%s", echo_args->sval[i]);
    }

    /* Print newline unless -n was specified */
    if (echo_no_newline->count == 0) {
        printf("\n");
    }

    /* Flush output to ensure it appears immediately */
    fflush(stdout);

    /* Clean up and return */
    arg_freetable(echo_argtable, 4);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

/**
 * Print usage/help information for echo
 *
 * This function is called when user runs "echo --help"
 * or when there's a usage error.
 *
 * @param out Output stream (stdout for --help, stderr for errors)
 */
void echo_print_usage(FILE *out)
{
    build_echo_argtable();

    fprintf(out, "Usage: echo ");
    arg_print_syntax(out, echo_argtable, "\n");
    fprintf(out, "Print arguments to standard output.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, echo_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  echo hello world           Print 'hello world' with newline\n");
    fprintf(out, "  echo -n \"no newline\"        Print without trailing newline\n");
    fprintf(out, "  echo --help                Show this help message\n");

    arg_freetable(echo_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

/**
 * Command specification structure for echo
 *
 * This structure is used by the command registry to:
 *   - Look up the command by name
 *   - Display help information
 *   - Execute the command
 */
cmd_spec_t cmd_echo_spec = {
    .name = "echo",
    .summary = "display a line of text",
    .long_help = "Print STRINGS to standard output. "
                 "With -n, do not output the trailing newline. "
                 "This is a refactored version using argtable3.",
    .run = echo_run,
    .print_usage = echo_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

/**
 * Register the echo command in the global registry
 *
 * This function should be called during shell initialization
 * to make the echo command available.
 */
void register_echo_command(void)
{
    register_command(&cmd_echo_spec);
}

/* ===== SECTION 7: STANDALONE MAIN (optional) ===== */

/**
 * Standalone main function for building echo as a separate binary
 *
 * This allows echo to be compiled as:
 *   1. A built-in shell command (when BUILTIN_ONLY is defined)
 *   2. A standalone executable (when BUILTIN_ONLY is not defined)
 *
 * Compile standalone:
 *   gcc -o echo cmd_echo.c registry.c -largtable3 -I../include
 *
 * Compile as built-in:
 *   gcc -DBUILTIN_ONLY -c cmd_echo.c
 */
#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_echo_spec.run(argc, argv);
}
#endif
