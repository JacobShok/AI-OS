/*
 * cmd_ln.c - Create links (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: ln [OPTIONS] TARGET LINK_NAME
 * Options:
 *   -s, --symbolic   Make symbolic links instead of hard links
 *   -f, --force      Remove existing destination files
 *   -h, --help       Display help message
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int ln_run(int argc, char **argv);
void ln_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *ln_help;
static struct arg_lit *ln_symbolic;
static struct arg_lit *ln_force;
static struct arg_file *ln_files;
static struct arg_end *ln_end;
static void *ln_argtable[6];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_ln_argtable(void)
{
    ln_help = arg_lit0("h", "help", "display this help and exit");
    ln_symbolic = arg_lit0("s", "symbolic", "make symbolic links instead of hard links");
    ln_force = arg_lit0("f", "force", "remove existing destination files");
    ln_files = arg_filen(NULL, NULL, "FILE", 2, 2, "target and link name");
    ln_end = arg_end(20);

    ln_argtable[0] = ln_help;
    ln_argtable[1] = ln_symbolic;
    ln_argtable[2] = ln_force;
    ln_argtable[3] = ln_files;
    ln_argtable[4] = ln_end;
    ln_argtable[5] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int ln_run(int argc, char **argv)
{
    int nerrors;
    int symbolic = 0;
    int force = 0;
    const char *target;
    const char *linkname;

    build_ln_argtable();
    nerrors = arg_parse(argc, argv, ln_argtable);

    /* Handle --help */
    if (ln_help->count > 0) {
        ln_print_usage(stdout);
        arg_freetable(ln_argtable, 5);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, ln_end, "ln");
        fprintf(stderr, "Try 'ln --help' for more information.\n");
        arg_freetable(ln_argtable, 5);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check flags */
    if (ln_symbolic->count > 0) {
        symbolic = 1;
    }
    if (ln_force->count > 0) {
        force = 1;
    }

    /* Get target and linkname */
    target = ln_files->filename[0];
    linkname = ln_files->filename[1];

    /* Remove existing link if -f */
    if (force) {
        unlink(linkname);
    }

    /* Create link */
    if (symbolic) {
        if (symlink(target, linkname) != 0) {
            perror("ln");
            arg_freetable(ln_argtable, 5);
            return EXIT_ERROR;
        }
    } else {
        if (link(target, linkname) != 0) {
            perror("ln");
            arg_freetable(ln_argtable, 5);
            return EXIT_ERROR;
        }
    }

    arg_freetable(ln_argtable, 5);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void ln_print_usage(FILE *out)
{
    build_ln_argtable();

    fprintf(out, "Usage: ln ");
    arg_print_syntax(out, ln_argtable, "\n");
    fprintf(out, "Create a link to TARGET with the name LINK_NAME.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, ln_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  ln file.txt link.txt       Create hard link\n");
    fprintf(out, "  ln -s file.txt link.txt    Create symbolic link\n");
    fprintf(out, "  ln -sf file.txt link.txt   Force create symbolic link\n");

    arg_freetable(ln_argtable, 5);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_ln_spec = {
    .name = "ln",
    .summary = "make links between files",
    .long_help = "Create a link to TARGET with the name LINK_NAME.",
    .run = ln_run,
    .print_usage = ln_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_ln_command(void)
{
    register_command(&cmd_ln_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_ln_spec.run(argc, argv);
}
#endif
