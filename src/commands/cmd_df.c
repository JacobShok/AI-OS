/*
 * cmd_df.c - Report filesystem disk space usage (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: df [OPTIONS] [FILE]
 * Options:
 *   -h, --human-readable   Print sizes in human readable format
 *   --help                 Display help message
 */

#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int df_run(int argc, char **argv);
void df_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *df_help;
static struct arg_lit *df_human;
static struct arg_file *df_path;
static struct arg_end *df_end;
static void *df_argtable[5];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_df_argtable(void)
{
    df_help = arg_lit0(NULL, "help", "display this help and exit");
    df_human = arg_lit0("h", "human-readable", "print sizes in human readable format");
    df_path = arg_file0(NULL, NULL, "FILE", "filesystem to check (default: current directory)");
    df_end = arg_end(20);

    df_argtable[0] = df_help;
    df_argtable[1] = df_human;
    df_argtable[2] = df_path;
    df_argtable[3] = df_end;
    df_argtable[4] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int df_run(int argc, char **argv)
{
    int nerrors;
    struct statvfs vfs;
    int human = 0;
    const char *path = ".";
    char size_buf[32];
    off_t total, avail, used;
    int use_percent;

    build_df_argtable();
    nerrors = arg_parse(argc, argv, df_argtable);

    /* Handle --help */
    if (df_help->count > 0) {
        df_print_usage(stdout);
        arg_freetable(df_argtable, 4);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, df_end, "df");
        fprintf(stderr, "Try 'df --help' for more information.\n");
        arg_freetable(df_argtable, 4);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check if human-readable flag was specified */
    if (df_human->count > 0) {
        human = 1;
    }

    /* Get path if provided */
    if (df_path->count > 0) {
        path = df_path->filename[0];
    }

    /* Get filesystem statistics */
    if (statvfs(path, &vfs) != 0) {
        perror("df");
        arg_freetable(df_argtable, 4);
        return EXIT_ERROR;
    }

    /* Print header */
    printf("Filesystem     ");
    if (human) {
        printf("Size  Used Avail Use%%\n");
    } else {
        printf("1K-blocks      Used Available Use%%\n");
    }

    /* Calculate statistics */
    total = vfs.f_blocks * vfs.f_frsize;
    avail = vfs.f_bavail * vfs.f_frsize;
    used = total - (vfs.f_bfree * vfs.f_frsize);
    use_percent = total > 0 ? (int)((used * 100) / total) : 0;

    /* Print filesystem info */
    printf("%-15s", path);

    if (human) {
        format_size(total, size_buf, sizeof(size_buf));
        printf("%5s ", size_buf);
        format_size(used, size_buf, sizeof(size_buf));
        printf("%5s ", size_buf);
        format_size(avail, size_buf, sizeof(size_buf));
        printf("%5s ", size_buf);
    } else {
        printf("%10lld %10lld %10lld ",
               (long long)(total / 1024),
               (long long)(used / 1024),
               (long long)(avail / 1024));
    }

    printf("%3d%%\n", use_percent);

    arg_freetable(df_argtable, 4);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void df_print_usage(FILE *out)
{
    build_df_argtable();

    fprintf(out, "Usage: df ");
    arg_print_syntax(out, df_argtable, "\n");
    fprintf(out, "Show information about the file system on which each FILE resides.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, df_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  df              Show filesystem info for current directory\n");
    fprintf(out, "  df -h           Show with human-readable sizes\n");
    fprintf(out, "  df /tmp         Show filesystem info for /tmp\n");

    arg_freetable(df_argtable, 4);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_df_spec = {
    .name = "df",
    .summary = "report file system disk space usage",
    .long_help = "Show information about the file system on which each FILE resides.",
    .run = df_run,
    .print_usage = df_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_df_command(void)
{
    register_command(&cmd_df_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_df_spec.run(argc, argv);
}
#endif
