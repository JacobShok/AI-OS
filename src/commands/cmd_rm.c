/*
 * cmd_rm.c - Remove files or directories (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: rm [OPTIONS] FILE...
 * Options:
 *   -r, -R, --recursive   Remove directories and contents recursively
 *   -f, --force           Force removal, ignore nonexistent files
 *   -h, --help            Display help message
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int rm_run(int argc, char **argv);
void rm_print_usage(FILE *out);
static int rm_recursive(const char *path);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *rm_help;
static struct arg_lit *rm_recursive_flag;
static struct arg_lit *rm_force;
static struct arg_file *rm_files;
static struct arg_end *rm_end;
static void *rm_argtable[6];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_rm_argtable(void)
{
    rm_help = arg_lit0("h", "help", "display this help and exit");
    rm_recursive_flag = arg_lit0("rR", "recursive", "remove directories and their contents recursively");
    rm_force = arg_lit0("f", "force", "force removal, ignore nonexistent files");
    rm_files = arg_filen(NULL, NULL, "FILE", 1, 100, "files to remove");
    rm_end = arg_end(20);

    rm_argtable[0] = rm_help;
    rm_argtable[1] = rm_recursive_flag;
    rm_argtable[2] = rm_force;
    rm_argtable[3] = rm_files;
    rm_argtable[4] = rm_end;
    rm_argtable[5] = NULL;
}

/* ===== HELPER FUNCTION ===== */

static int rm_recursive(const char *path)
{
    DIR *dir;
    struct dirent *entry;
    char filepath[4096];
    struct stat st;

    if (lstat(path, &st) != 0) {
        perror(path);
        return EXIT_ERROR;
    }

    if (!S_ISDIR(st.st_mode)) {
        if (unlink(path) != 0) {
            perror(path);
            return EXIT_ERROR;
        }
        return EXIT_OK;
    }

    /* Remove directory contents */
    dir = opendir(path);
    if (!dir) {
        perror(path);
        return EXIT_ERROR;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        rm_recursive(filepath);
    }

    closedir(dir);

    /* Remove the directory itself */
    if (rmdir(path) != 0) {
        perror(path);
        return EXIT_ERROR;
    }

    return EXIT_OK;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int rm_run(int argc, char **argv)
{
    int nerrors;
    int recursive = 0;
    int force = 0;
    int i;
    int ret = EXIT_OK;

    build_rm_argtable();
    nerrors = arg_parse(argc, argv, rm_argtable);

    /* Handle --help */
    if (rm_help->count > 0) {
        rm_print_usage(stdout);
        arg_freetable(rm_argtable, 5);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, rm_end, "rm");
        fprintf(stderr, "Try 'rm --help' for more information.\n");
        arg_freetable(rm_argtable, 5);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check flags */
    if (rm_recursive_flag->count > 0) {
        recursive = 1;
    }
    if (rm_force->count > 0) {
        force = 1;
    }

    /* Remove each file */
    for (i = 0; i < rm_files->count; i++) {
        const char *path = rm_files->filename[i];

        if (is_directory(path)) {
            if (!recursive) {
                fprintf(stderr, "rm: '%s' is a directory (use -r)\n", path);
                ret = EXIT_ERROR;
                continue;
            }
            if (rm_recursive(path) != EXIT_OK && !force) {
                ret = EXIT_ERROR;
            }
        } else {
            if (unlink(path) != 0) {
                if (!force) {
                    perror(path);
                    ret = EXIT_ERROR;
                }
            }
        }
    }

    arg_freetable(rm_argtable, 5);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void rm_print_usage(FILE *out)
{
    build_rm_argtable();

    fprintf(out, "Usage: rm ");
    arg_print_syntax(out, rm_argtable, "\n");
    fprintf(out, "Remove (unlink) the FILE(s).\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, rm_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  rm file.txt               Remove file.txt\n");
    fprintf(out, "  rm -r mydir               Remove directory and contents\n");
    fprintf(out, "  rm -f file.txt            Force removal, ignore errors\n");
    fprintf(out, "  rm file1.txt file2.txt    Remove multiple files\n");

    arg_freetable(rm_argtable, 5);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_rm_spec = {
    .name = "rm",
    .summary = "remove files or directories",
    .long_help = "Remove (unlink) the FILE(s).",
    .run = rm_run,
    .print_usage = rm_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_rm_command(void)
{
    register_command(&cmd_rm_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_rm_spec.run(argc, argv);
}
#endif
