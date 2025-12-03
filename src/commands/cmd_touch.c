/*
 * cmd_touch.c - Create empty files or update timestamps (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: touch [OPTIONS] FILE...
 * Options:
 *   -c, --no-create   Do not create file if it doesn't exist
 *   -h, --help        Display help message
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <unistd.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int touch_run(int argc, char **argv);
void touch_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *touch_help;
static struct arg_lit *touch_no_create;
static struct arg_file *touch_files;
static struct arg_end *touch_end;
static void *touch_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_touch_argtable(void)
{
    touch_help = arg_lit0("h", "help", "display this help and exit");
    touch_no_create = arg_lit0("c", "no-create", "do not create any files");
    touch_files = arg_filen(NULL, NULL, "FILE", 1, 100, "files to touch");
    touch_end = arg_end(20);

    touch_argtable[0] = touch_help;
    touch_argtable[1] = touch_no_create;
    touch_argtable[2] = touch_files;
    touch_argtable[3] = touch_end;
    touch_argtable[4] = NULL;
}

/* ===== HELPER FUNCTION ===== */

/*
 * Touch a single file - create if doesn't exist, update timestamp if it does
 */
static int touch_file(const char *filename, int no_create)
{
    int fd;
    struct stat st;

    /* Check if file exists */
    if (stat(filename, &st) == 0) {
        /* File exists - update its timestamp */
        if (utime(filename, NULL) != 0) {
            perror(filename);
            return EXIT_ERROR;
        }
        return EXIT_OK;
    }

    /* File doesn't exist */
    if (no_create) {
        /* -c flag: don't create, just silently succeed */
        return EXIT_OK;
    }

    /* Create the file */
    fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, 0666);
    if (fd < 0) {
        perror(filename);
        return EXIT_ERROR;
    }

    close(fd);
    return EXIT_OK;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int touch_run(int argc, char **argv)
{
    int nerrors;
    int no_create = 0;
    int i;
    int ret = EXIT_OK;

    build_touch_argtable();
    nerrors = arg_parse(argc, argv, touch_argtable);

    /* Handle --help */
    if (touch_help->count > 0) {
        touch_print_usage(stdout);
        arg_freetable(touch_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, touch_end, "touch");
        fprintf(stderr, "Try 'touch --help' for more information.\n");
        arg_freetable(touch_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check if -c flag was specified */
    if (touch_no_create->count > 0) {
        no_create = 1;
    }

    /* Touch each file */
    for (i = 0; i < touch_files->count; i++) {
        if (touch_file(touch_files->filename[i], no_create) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    arg_freetable(touch_argtable, 4);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void touch_print_usage(FILE *out)
{
    build_touch_argtable();

    fprintf(out, "Usage: touch ");
    arg_print_syntax(out, touch_argtable, "\n");
    fprintf(out, "Update the access and modification times of each FILE to the current time.\n");
    fprintf(out, "A FILE argument that does not exist is created empty, unless -c is supplied.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, touch_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  touch file.txt         Create file.txt or update its timestamp\n");
    fprintf(out, "  touch -c existing.txt  Update timestamp only if file exists\n");
    fprintf(out, "  touch f1.txt f2.txt    Touch multiple files\n");

    arg_freetable(touch_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_touch_spec = {
    .name = "touch",
    .summary = "change file timestamps",
    .long_help = "Update the access and modification times of each FILE to the current time. "
                 "A FILE argument that does not exist is created empty, unless -c is supplied.",
    .run = touch_run,
    .print_usage = touch_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_touch_command(void)
{
    register_command(&cmd_touch_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_touch_spec.run(argc, argv);
}
#endif
