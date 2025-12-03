/*
 * cmd_mkdir.c - Create directories (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: mkdir [OPTIONS] DIRECTORY...
 * Options:
 *   -p, --parents      Create parent directories as needed
 *   -m, --mode=MODE    Set file mode (permissions)
 *   -h, --help         Display help message
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int mkdir_run(int argc, char **argv);
void mkdir_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *mkdir_help;
static struct arg_lit *mkdir_parents;
static struct arg_str *mkdir_mode;
static struct arg_file *mkdir_dirs;
static struct arg_end *mkdir_end;
static void *mkdir_argtable[6];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_mkdir_argtable(void)
{
    mkdir_help = arg_lit0("h", "help", "display this help and exit");
    mkdir_parents = arg_lit0("p", "parents", "make parent directories as needed");
    mkdir_mode = arg_str0("m", "mode", "MODE", "set file mode (as in chmod)");
    mkdir_dirs = arg_filen(NULL, NULL, "DIRECTORY", 1, 100, "directories to create");
    mkdir_end = arg_end(20);

    mkdir_argtable[0] = mkdir_help;
    mkdir_argtable[1] = mkdir_parents;
    mkdir_argtable[2] = mkdir_mode;
    mkdir_argtable[3] = mkdir_dirs;
    mkdir_argtable[4] = mkdir_end;
    mkdir_argtable[5] = NULL;
}

/* ===== HELPER FUNCTIONS ===== */

/*
 * Parse octal mode string (e.g., "755" -> 0755)
 */
static int parse_mode(const char *mode_str, mode_t *mode)
{
    char *endptr;
    long val;

    val = strtol(mode_str, &endptr, 8);  /* Parse as octal */
    if (*endptr != '\0' || val < 0 || val > 0777) {
        fprintf(stderr, "mkdir: invalid mode '%s'\n", mode_str);
        return -1;
    }

    *mode = (mode_t)val;
    return 0;
}

/*
 * Create a single directory with given mode
 */
static int create_single_dir(const char *path, mode_t mode)
{
    if (mkdir(path, mode) != 0) {
        if (errno != EEXIST) {
            perror(path);
            return EXIT_ERROR;
        }
        /* Directory already exists - not an error with -p */
    }
    return EXIT_OK;
}

/*
 * Create directory and all parents as needed
 * Example: "a/b/c" creates "a", then "a/b", then "a/b/c"
 */
static int create_parents(const char *path, mode_t mode)
{
    char *path_copy;
    char *p;
    int ret = EXIT_OK;

    /* Make a copy we can modify */
    path_copy = strdup(path);
    if (path_copy == NULL) {
        perror("mkdir");
        return EXIT_ERROR;
    }

    /* Create each directory in the path */
    for (p = path_copy + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            /* Create this level */
            if (create_single_dir(path_copy, mode) != EXIT_OK) {
                /* Only fail if it's not EEXIST */
                if (errno != EEXIST) {
                    ret = EXIT_ERROR;
                    goto cleanup;
                }
            }

            /* Restore the slash */
            *p = '/';
        }
    }

    /* Create the final directory */
    if (create_single_dir(path_copy, mode) != EXIT_OK) {
        ret = EXIT_ERROR;
    }

cleanup:
    free(path_copy);
    return ret;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int mkdir_run(int argc, char **argv)
{
    int nerrors;
    int parents = 0;
    mode_t mode = 0777;  /* Default: rwxrwxrwx (modified by umask) */
    int i;
    int ret = EXIT_OK;

    build_mkdir_argtable();
    nerrors = arg_parse(argc, argv, mkdir_argtable);

    /* Handle --help */
    if (mkdir_help->count > 0) {
        mkdir_print_usage(stdout);
        arg_freetable(mkdir_argtable, 5);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, mkdir_end, "mkdir");
        fprintf(stderr, "Try 'mkdir --help' for more information.\n");
        arg_freetable(mkdir_argtable, 5);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check if -p flag was specified */
    if (mkdir_parents->count > 0) {
        parents = 1;
    }

    /* Parse mode if specified */
    if (mkdir_mode->count > 0) {
        if (parse_mode(mkdir_mode->sval[0], &mode) != 0) {
            arg_freetable(mkdir_argtable, 5);
            return EXIT_ERROR;
        }
    }

    /* Create each directory */
    for (i = 0; i < mkdir_dirs->count; i++) {
        if (parents) {
            if (create_parents(mkdir_dirs->filename[i], mode) != EXIT_OK) {
                ret = EXIT_ERROR;
            }
        } else {
            if (create_single_dir(mkdir_dirs->filename[i], mode) != EXIT_OK) {
                ret = EXIT_ERROR;
            }
        }
    }

    arg_freetable(mkdir_argtable, 5);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void mkdir_print_usage(FILE *out)
{
    build_mkdir_argtable();

    fprintf(out, "Usage: mkdir ");
    arg_print_syntax(out, mkdir_argtable, "\n");
    fprintf(out, "Create the DIRECTORY(ies), if they do not already exist.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, mkdir_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  mkdir newdir           Create directory 'newdir'\n");
    fprintf(out, "  mkdir -p a/b/c         Create nested directories\n");
    fprintf(out, "  mkdir -m 755 mydir     Create with specific permissions\n");

    arg_freetable(mkdir_argtable, 5);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_mkdir_spec = {
    .name = "mkdir",
    .summary = "make directories",
    .long_help = "Create the DIRECTORY(ies), if they do not already exist.",
    .run = mkdir_run,
    .print_usage = mkdir_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_mkdir_command(void)
{
    register_command(&cmd_mkdir_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_mkdir_spec.run(argc, argv);
}
#endif
