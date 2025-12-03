/*
 * cmd_pwd.c - Print current working directory (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: pwd [OPTIONS]
 * Options:
 *   -L, --logical     Use PWD from environment (logical path with symlinks)
 *   -P, --physical    Print physical directory (resolve all symlinks) [default]
 *   -h, --help        Display help message
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int pwd_run(int argc, char **argv);
void pwd_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES (static to this file) ===== */

static struct arg_lit *pwd_help;
static struct arg_lit *pwd_logical;
static struct arg_lit *pwd_physical;
static struct arg_end *pwd_end;
static void *pwd_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

/**
 * Build the argtable definition for pwd command
 *
 * This defines the command-line interface:
 *   -h, --help      Display help
 *   -L, --logical   Use PWD environment variable
 *   -P, --physical  Use getcwd() system call (default)
 */
static void build_pwd_argtable(void)
{
    /* Define help flag */
    pwd_help = arg_lit0("h", "help", "display this help and exit");

    /* Define logical path flag */
    pwd_logical = arg_lit0("L", "logical", "use PWD from environment, even if it contains symlinks");

    /* Define physical path flag (default behavior) */
    pwd_physical = arg_lit0("P", "physical", "avoid all symlinks (default)");

    /* Error handling structure */
    pwd_end = arg_end(20);

    /* Build the argtable array */
    pwd_argtable[0] = pwd_help;
    pwd_argtable[1] = pwd_logical;
    pwd_argtable[2] = pwd_physical;
    pwd_argtable[3] = pwd_end;
    pwd_argtable[4] = NULL;  /* Always NULL-terminate */
}

/* ===== SECTION 3: RUN FUNCTION (main implementation) ===== */

/**
 * Execute the pwd command
 *
 * Prints the current working directory to stdout.
 * With -L, uses PWD environment variable (logical path).
 * With -P or by default, uses getcwd() (physical path).
 *
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return EXIT_OK on success, EXIT_ERROR on failure
 */
int pwd_run(int argc, char **argv)
{
    int nerrors;
    int use_logical;
    char cwd[PATH_MAX];
    char *pwd_env;

    /* Build argtable definition */
    build_pwd_argtable();

    /* Parse command-line arguments */
    nerrors = arg_parse(argc, argv, pwd_argtable);

    /* Handle --help FIRST (before checking for errors) */
    if (pwd_help->count > 0) {
        pwd_print_usage(stdout);
        arg_freetable(pwd_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, pwd_end, "pwd");
        fprintf(stderr, "Try 'pwd --help' for more information.\n");
        arg_freetable(pwd_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Determine if we should use logical path
     * Default is physical (-P), unless -L is specified */
    use_logical = (pwd_logical->count > 0);

    /* If -P is explicitly specified, it overrides -L
     * (last flag wins, but we'll just honor -P if both are given) */
    if (pwd_physical->count > 0 && pwd_logical->count > 0) {
        /* If both specified, -P wins (matches standard pwd behavior) */
        use_logical = 0;
    }

    /* If -L, try to use PWD environment variable */
    if (use_logical) {
        pwd_env = getenv("PWD");
        if (pwd_env != NULL && pwd_env[0] != '\0') {
            printf("%s\n", pwd_env);
            arg_freetable(pwd_argtable, 4);
            return EXIT_OK;
        }
        /* Fall through to getcwd() if PWD not set or empty */
    }

    /* Get current working directory using system call
     * This is the default behavior and also fallback for -L */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd");
        arg_freetable(pwd_argtable, 4);
        return EXIT_ERROR;
    }

    printf("%s\n", cwd);

    /* Clean up and return */
    arg_freetable(pwd_argtable, 4);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

/**
 * Print usage/help information for pwd
 *
 * This function is called when user runs "pwd --help"
 * or when there's a usage error.
 *
 * @param out Output stream (stdout for --help, stderr for errors)
 */
void pwd_print_usage(FILE *out)
{
    build_pwd_argtable();

    fprintf(out, "Usage: pwd ");
    arg_print_syntax(out, pwd_argtable, "\n");
    fprintf(out, "Print the full filename of the current working directory.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, pwd_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  pwd                        Print physical current directory\n");
    fprintf(out, "  pwd -L                     Print logical current directory (with symlinks)\n");
    fprintf(out, "  pwd -P                     Print physical current directory (resolve symlinks)\n");

    arg_freetable(pwd_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

/**
 * Command specification structure for pwd
 *
 * This structure is used by the command registry to:
 *   - Look up the command by name
 *   - Display help information
 *   - Execute the command
 */
cmd_spec_t cmd_pwd_spec = {
    .name = "pwd",
    .summary = "print name of current/working directory",
    .long_help = "Print the full filename of the current working directory. "
                 "With -L, use PWD from environment (even if it contains symlinks). "
                 "With -P (default), resolve all symlinks to get physical path.",
    .run = pwd_run,
    .print_usage = pwd_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

/**
 * Register the pwd command in the global registry
 *
 * This function should be called during shell initialization
 * to make the pwd command available.
 */
void register_pwd_command(void)
{
    register_command(&cmd_pwd_spec);
}

/* ===== SECTION 7: STANDALONE MAIN (optional) ===== */

/**
 * Standalone main function for building pwd as a separate binary
 *
 * Compile standalone:
 *   gcc -o pwd cmd_pwd.c registry.c -largtable3 -I../include
 *
 * Compile as built-in:
 *   gcc -DBUILTIN_ONLY -c cmd_pwd.c
 */
#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_pwd_spec.run(argc, argv);
}
#endif
