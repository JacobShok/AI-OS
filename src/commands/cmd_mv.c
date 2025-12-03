/*
 * cmd_mv.c - Move or rename files (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: mv [OPTIONS] SOURCE DEST
 * Options:
 *   -f, --force     Force overwrite (accepted but not fully implemented)
 *   -h, --help      Display help message
 */

#include <stdio.h>
#include <string.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int mv_run(int argc, char **argv);
void mv_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *mv_help;
static struct arg_lit *mv_force;
static struct arg_file *mv_files;
static struct arg_end *mv_end;
static void *mv_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_mv_argtable(void)
{
    mv_help = arg_lit0("h", "help", "display this help and exit");
    mv_force = arg_lit0("f", "force", "force overwrite");
    mv_files = arg_filen(NULL, NULL, "FILE", 2, 2, "source and destination");
    mv_end = arg_end(20);

    mv_argtable[0] = mv_help;
    mv_argtable[1] = mv_force;
    mv_argtable[2] = mv_files;
    mv_argtable[3] = mv_end;
    mv_argtable[4] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int mv_run(int argc, char **argv)
{
    int nerrors;
    const char *src;
    const char *dest;

    build_mv_argtable();
    nerrors = arg_parse(argc, argv, mv_argtable);

    /* Handle --help */
    if (mv_help->count > 0) {
        mv_print_usage(stdout);
        arg_freetable(mv_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, mv_end, "mv");
        fprintf(stderr, "Try 'mv --help' for more information.\n");
        arg_freetable(mv_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Get source and destination */
    src = mv_files->filename[0];
    dest = mv_files->filename[1];

    /* Try rename (works on same filesystem) */
    if (rename(src, dest) != 0) {
        perror("mv");
        arg_freetable(mv_argtable, 4);
        return EXIT_ERROR;
    }

    arg_freetable(mv_argtable, 4);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void mv_print_usage(FILE *out)
{
    build_mv_argtable();

    fprintf(out, "Usage: mv ");
    arg_print_syntax(out, mv_argtable, "\n");
    fprintf(out, "Rename SOURCE to DEST, or move SOURCE to DIRECTORY.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, mv_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  mv file1.txt file2.txt    Rename file1.txt to file2.txt\n");
    fprintf(out, "  mv file.txt /tmp/         Move file.txt to /tmp/\n");
    fprintf(out, "  mv oldname newname        Rename oldname to newname\n");

    arg_freetable(mv_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_mv_spec = {
    .name = "mv",
    .summary = "move (rename) files",
    .long_help = "Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.",
    .run = mv_run,
    .print_usage = mv_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_mv_command(void)
{
    register_command(&cmd_mv_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_mv_spec.run(argc, argv);
}
#endif
