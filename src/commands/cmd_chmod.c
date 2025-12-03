/*
 * cmd_chmod.c - Change file permissions (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: chmod MODE FILE...
 * MODE is an octal number like 755 or 644.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int chmod_run(int argc, char **argv);
void chmod_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *chmod_help;
static struct arg_str *chmod_mode;
static struct arg_file *chmod_files;
static struct arg_end *chmod_end;
static void *chmod_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_chmod_argtable(void)
{
    chmod_help = arg_lit0("h", "help", "display this help and exit");
    chmod_mode = arg_str1(NULL, NULL, "MODE", "octal mode (e.g., 755, 644)");
    chmod_files = arg_filen(NULL, NULL, "FILE", 1, 100, "files to change mode");
    chmod_end = arg_end(20);

    chmod_argtable[0] = chmod_help;
    chmod_argtable[1] = chmod_mode;
    chmod_argtable[2] = chmod_files;
    chmod_argtable[3] = chmod_end;
    chmod_argtable[4] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int chmod_run(int argc, char **argv)
{
    int nerrors;
    mode_t mode;
    char *endptr;
    long val;
    int i;
    int ret = EXIT_OK;

    build_chmod_argtable();
    nerrors = arg_parse(argc, argv, chmod_argtable); // updates argtable with argv

    /* Handle --help */
    if (chmod_help->count > 0) {
        chmod_print_usage(stdout);
        arg_freetable(chmod_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, chmod_end, "chmod");
        fprintf(stderr, "Try 'chmod --help' for more information.\n");
        arg_freetable(chmod_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Parse mode (octal) */
    val = strtol(chmod_mode->sval[0], &endptr, 8);
    if (*endptr != '\0' || val < 0 || val > 0777) {
        fprintf(stderr, "chmod: invalid mode '%s'\n", chmod_mode->sval[0]);
        arg_freetable(chmod_argtable, 4);
        return EXIT_ERROR;
    }
    mode = (mode_t)val;

    /* Change mode for each file */
    for (i = 0; i < chmod_files->count; i++) {
        if (chmod(chmod_files->filename[i], mode) != 0) {
            perror(chmod_files->filename[i]);
            ret = EXIT_ERROR;
        }
    }

    arg_freetable(chmod_argtable, 4);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void chmod_print_usage(FILE *out)
{
    build_chmod_argtable();

    fprintf(out, "Usage: chmod ");
    arg_print_syntax(out, chmod_argtable, "\n");
    fprintf(out, "Change the mode of each FILE to MODE.\n");
    fprintf(out, "MODE is an octal number like 755 or 644.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, chmod_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  chmod 755 script.sh   Make file rwxr-xr-x\n");
    fprintf(out, "  chmod 644 file.txt    Make file rw-r--r--\n");
    fprintf(out, "  chmod 600 secret.txt  Make file rw-------\n");

    arg_freetable(chmod_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_chmod_spec = {
    .name = "chmod",
    .summary = "change file mode bits",
    .long_help = "Change the mode of each FILE to MODE.",
    .run = chmod_run,
    .print_usage = chmod_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_chmod_command(void)
{
    register_command(&cmd_chmod_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_chmod_spec.run(argc, argv);
}
#endif
