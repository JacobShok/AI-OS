/*
 * cmd_stat.c - Display file status (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: stat FILE...
 * Display file or file system status.
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int stat_run(int argc, char **argv);
void stat_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *stat_help;
static struct arg_file *stat_files;
static struct arg_end *stat_end;
static void *stat_argtable[4];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_stat_argtable(void)
{
    stat_help = arg_lit0("h", "help", "display this help and exit");
    stat_files = arg_filen(NULL, NULL, "FILE", 1, 100, "files to stat");
    stat_end = arg_end(20);

    stat_argtable[0] = stat_help;
    stat_argtable[1] = stat_files;
    stat_argtable[2] = stat_end;
    stat_argtable[3] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int stat_run(int argc, char **argv)
{
    int nerrors;
    struct stat st;
    int i;
    int ret = EXIT_OK;
    char time_buf[64];

    build_stat_argtable();
    nerrors = arg_parse(argc, argv, stat_argtable);

    /* Handle --help */
    if (stat_help->count > 0) {
        stat_print_usage(stdout);
        arg_freetable(stat_argtable, 3);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, stat_end, "stat");
        fprintf(stderr, "Try 'stat --help' for more information.\n");
        arg_freetable(stat_argtable, 3);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Process each file */
    for (i = 0; i < stat_files->count; i++) {
        const char *filename = stat_files->filename[i];

        if (stat(filename, &st) != 0) {
            perror(filename);
            ret = EXIT_ERROR;
            continue;
        }

        printf("  File: %s\n", filename);
        printf("  Size: %lld\n", (long long)st.st_size);
        printf("Blocks: %lld\n", (long long)st.st_blocks);
        printf("  Mode: %04o\n", st.st_mode & 0777);
        printf("  Uid: %d\n", st.st_uid);
        printf("  Gid: %d\n", st.st_gid);

        format_time(st.st_atime, time_buf, sizeof(time_buf));
        printf("Access: %s\n", time_buf);

        format_time(st.st_mtime, time_buf, sizeof(time_buf));
        printf("Modify: %s\n", time_buf);

        format_time(st.st_ctime, time_buf, sizeof(time_buf));
        printf("Change: %s\n", time_buf);
    }

    arg_freetable(stat_argtable, 3);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void stat_print_usage(FILE *out)
{
    build_stat_argtable();

    fprintf(out, "Usage: stat ");
    arg_print_syntax(out, stat_argtable, "\n");
    fprintf(out, "Display file or file system status.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, stat_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  stat file.txt            Display status of file.txt\n");
    fprintf(out, "  stat file1.txt file2.txt Display status of multiple files\n");

    arg_freetable(stat_argtable, 3);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_stat_spec = {
    .name = "stat",
    .summary = "display file or file system status",
    .long_help = "Display file or file system status.",
    .run = stat_run,
    .print_usage = stat_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_stat_command(void)
{
    register_command(&cmd_stat_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_stat_spec.run(argc, argv);
}
#endif
