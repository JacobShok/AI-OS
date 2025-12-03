/*
 * cmd_cp.c - Copy files and directories (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: cp [OPTIONS] SOURCE DEST
 * Options:
 *   -r, -R, --recursive   Copy directories recursively
 *   -f, --force           Force overwrite (accepted but not fully implemented)
 *   -h, --help            Display help message
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int cp_run(int argc, char **argv);
void cp_print_usage(FILE *out);
static int cp_recursive(const char *src, const char *dest);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *cp_help;
static struct arg_lit *cp_recursive_flag;
static struct arg_lit *cp_force;
static struct arg_file *cp_files;
static struct arg_end *cp_end;
static void *cp_argtable[6];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_cp_argtable(void)
{
    cp_help = arg_lit0("h", "help", "display this help and exit");
    cp_recursive_flag = arg_lit0("rR", "recursive", "copy directories recursively");
    cp_force = arg_lit0("f", "force", "force overwrite");
    cp_files = arg_filen(NULL, NULL, "FILE", 2, 2, "source and destination");
    cp_end = arg_end(20);

    cp_argtable[0] = cp_help;
    cp_argtable[1] = cp_recursive_flag;
    cp_argtable[2] = cp_force;
    cp_argtable[3] = cp_files;
    cp_argtable[4] = cp_end;
    cp_argtable[5] = NULL;
}

/* ===== HELPER FUNCTIONS ===== */

static int cp_file_to_file(const char *src, const char *dest)
{
    if (copy_file(src, dest) < 0) {
        perror(dest);
        return EXIT_ERROR;
    }
    return EXIT_OK;
}

static int cp_recursive(const char *src, const char *dest)
{
    DIR *dir;
    struct dirent *entry;
    char src_path[4096];
    char dest_path[4096];
    struct stat st;

    if (stat(src, &st) != 0) {
        perror(src);
        return EXIT_ERROR;
    }

    if (!S_ISDIR(st.st_mode)) {
        return cp_file_to_file(src, dest);
    }

    /* Create destination directory */
    if (mkdir(dest, st.st_mode) != 0 && errno != EEXIST) {
        perror(dest);
        return EXIT_ERROR;
    }

    /* Copy directory contents */
    dir = opendir(src);
    if (!dir) {
        perror(src);
        return EXIT_ERROR;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        cp_recursive(src_path, dest_path);
    }

    closedir(dir);
    return EXIT_OK;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int cp_run(int argc, char **argv)
{
    int nerrors;
    int recursive = 0;
    const char *src;
    const char *dest;

    build_cp_argtable();
    nerrors = arg_parse(argc, argv, cp_argtable);

    /* Handle --help */
    if (cp_help->count > 0) {
        cp_print_usage(stdout);
        arg_freetable(cp_argtable, 5);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, cp_end, "cp");
        fprintf(stderr, "Try 'cp --help' for more information.\n");
        arg_freetable(cp_argtable, 5);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check if recursive flag was specified */
    if (cp_recursive_flag->count > 0) {
        recursive = 1;
    }

    /* Get source and destination */
    src = cp_files->filename[0];
    dest = cp_files->filename[1];

    /* If source is a directory but -r wasn't specified → error */
    if (is_directory(src) && !recursive) {
        fprintf(stderr, "cp: '%s' is a directory (use -r)\n", src);
        arg_freetable(cp_argtable, 5);
        return EXIT_ERROR;
    }

    /* Perform the copy */
    int ret;
    if (recursive) {
        ret = cp_recursive(src, dest);
    } else {
        ret = cp_file_to_file(src, dest);
    }

    arg_freetable(cp_argtable, 5);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void cp_print_usage(FILE *out)
{
    build_cp_argtable();

    fprintf(out, "Usage: cp ");
    arg_print_syntax(out, cp_argtable, "\n");
    fprintf(out, "Copy SOURCE to DEST.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, cp_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  cp file1.txt file2.txt        Copy single file\n");
    fprintf(out, "  cp script.sh /usr/local/bin/  Copy file to directory\n");
    fprintf(out, "  cp -r mydir backup/           Copy directory recursively\n");

    arg_freetable(cp_argtable, 5);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_cp_spec = {
    .name = "cp",
    .summary = "copy files and directories",
    .long_help = "Copy SOURCE to DEST, or multiple SOURCE(s) to DIRECTORY.",
    .run = cp_run,
    .print_usage = cp_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_cp_command(void)
{
    register_command(&cmd_cp_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_cp_spec.run(argc, argv);
}
#endif
