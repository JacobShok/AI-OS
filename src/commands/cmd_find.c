/*
 * cmd_find.c - Search for files (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: find [PATH] [OPTIONS]
 * Options:
 *   -name PATTERN   Base of file name matches PATTERN
 *   -type TYPE      File is of type TYPE (f=file, d=directory)
 *   -h, --help      Display help message
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int find_run(int argc, char **argv);
void find_print_usage(FILE *out);
static void find_recursive(const char *path, const char *name_pattern, char type_filter);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *find_help;
static struct arg_str *find_name;
static struct arg_str *find_type;
static struct arg_file *find_path;
static struct arg_end *find_end;
static void *find_argtable[6];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_find_argtable(void)
{
    find_help = arg_lit0("h", "help", "display this help and exit");
    find_name = arg_str0(NULL, "name", "PATTERN", "base of file name matches PATTERN");
    find_type = arg_str0(NULL, "type", "TYPE", "file is of type TYPE (f=file, d=directory)");
    find_path = arg_file0(NULL, NULL, "PATH", "starting directory (default: current)");
    find_end = arg_end(20);

    find_argtable[0] = find_help;
    find_argtable[1] = find_name;
    find_argtable[2] = find_type;
    find_argtable[3] = find_path;
    find_argtable[4] = find_end;
    find_argtable[5] = NULL;
}

/* ===== HELPER FUNCTION ===== */

static void find_recursive(const char *path, const char *name_pattern, char type_filter)
{
    DIR *dir;
    struct dirent *entry;
    char filepath[4096];
    struct stat st;

    dir = opendir(path);
    if (!dir) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (lstat(filepath, &st) != 0) {
            continue;
        }

        int match = 1;

        /* Check name pattern */
        if (name_pattern && fnmatch(name_pattern, entry->d_name, 0) != 0) {
            match = 0;
        }

        /* Check type filter */
        if (type_filter) {
            if (type_filter == 'f' && !S_ISREG(st.st_mode)) {
                match = 0;
            } else if (type_filter == 'd' && !S_ISDIR(st.st_mode)) {
                match = 0;
            }
        }

        if (match) {
            printf("%s\n", filepath);
        }

        /* Recurse into directories */
        if (S_ISDIR(st.st_mode)) {
            find_recursive(filepath, name_pattern, type_filter);
        }
    }

    closedir(dir);
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int find_run(int argc, char **argv)
{
    int nerrors;
    const char *start_path = ".";
    const char *name_pattern = NULL;
    char type_filter = 0;

    build_find_argtable();
    nerrors = arg_parse(argc, argv, find_argtable);

    /* Handle --help */
    if (find_help->count > 0) {
        find_print_usage(stdout);
        arg_freetable(find_argtable, 5);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, find_end, "find");
        fprintf(stderr, "Try 'find --help' for more information.\n");
        arg_freetable(find_argtable, 5);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Get path if specified */
    if (find_path->count > 0) {
        start_path = find_path->filename[0];
    }

    /* Get name pattern if specified */
    if (find_name->count > 0) {
        name_pattern = find_name->sval[0];
    }

    /* Get type filter if specified */
    if (find_type->count > 0) {
        type_filter = find_type->sval[0][0];
    }

    /* Perform the find */
    find_recursive(start_path, name_pattern, type_filter);

    arg_freetable(find_argtable, 5);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void find_print_usage(FILE *out)
{
    build_find_argtable();

    fprintf(out, "Usage: find ");
    arg_print_syntax(out, find_argtable, "\n");
    fprintf(out, "Search for files in a directory hierarchy.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, find_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Pattern Matching:\n");
    fprintf(out, "  *.c          Files ending with .c\n");
    fprintf(out, "  cmd_*        Files starting with cmd_\n");
    fprintf(out, "  *test*       Files containing 'test'\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  find                     List all files in current directory\n");
    fprintf(out, "  find /tmp                List all files in /tmp\n");
    fprintf(out, "  find --name '*.c'        Find all .c files\n");
    fprintf(out, "  find --type f            Find only regular files\n");
    fprintf(out, "  find --type d            Find only directories\n");

    arg_freetable(find_argtable, 5);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_find_spec = {
    .name = "find",
    .summary = "search for files in a directory hierarchy",
    .long_help = "Search for files in a directory hierarchy.",
    .run = find_run,
    .print_usage = find_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_find_command(void)
{
    register_command(&cmd_find_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_find_spec.run(argc, argv);
}
#endif
