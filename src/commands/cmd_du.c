/*
 * cmd_du.c - Estimate file space usage (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: du [OPTIONS] [FILE...]
 * Options:
 *   -h, --human-readable   Print sizes in human readable format
 *   -s, --summarize        Display only a total for each argument
 *   --help                 Display help message
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int du_run(int argc, char **argv);
void du_print_usage(FILE *out);
static off_t du_recursive(const char *path, int summary, int human);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *du_help;
static struct arg_lit *du_human;
static struct arg_lit *du_summary;
static struct arg_file *du_paths;
static struct arg_end *du_end;
static void *du_argtable[6];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_du_argtable(void)
{
    du_help = arg_lit0(NULL, "help", "display this help and exit");
    du_human = arg_lit0("h", "human-readable", "print sizes in human readable format");
    du_summary = arg_lit0("s", "summarize", "display only a total for each argument");
    du_paths = arg_filen(NULL, NULL, "FILE", 0, 100, "files/directories to check");
    du_end = arg_end(20);

    du_argtable[0] = du_help;
    du_argtable[1] = du_human;
    du_argtable[2] = du_summary;
    du_argtable[3] = du_paths;
    du_argtable[4] = du_end;
    du_argtable[5] = NULL;
}

/* ===== HELPER FUNCTION ===== */

static off_t du_recursive(const char *path, int summary, int human)
{
    struct stat st;
    DIR *dir;
    struct dirent *entry;
    char filepath[4096];
    off_t total = 0;

    if (lstat(path, &st) != 0) {
        perror(path);
        return 0;
    }

    total = st.st_blocks * 512;  /* Convert blocks to bytes */

    if (S_ISDIR(st.st_mode)) {
        dir = opendir(path);
        if (!dir) {
            perror(path);
            return total;
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
            total += du_recursive(filepath, 1, human);
        }

        closedir(dir);
    }

    if (!summary) {
        char size_buf[32];
        if (human) {
            format_size(total, size_buf, sizeof(size_buf));
            printf("%s\t%s\n", size_buf, path);
        } else {
            printf("%lld\t%s\n", (long long)(total / 1024), path);
        }
    }

    return total;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int du_run(int argc, char **argv)
{
    int nerrors;
    int summary = 0;
    int human = 0;
    int i;
    off_t total;
    char size_buf[32];

    build_du_argtable();
    nerrors = arg_parse(argc, argv, du_argtable);

    /* Handle --help */
    if (du_help->count > 0) {
        du_print_usage(stdout);
        arg_freetable(du_argtable, 5);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, du_end, "du");
        fprintf(stderr, "Try 'du --help' for more information.\n");
        arg_freetable(du_argtable, 5);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check flags */
    if (du_human->count > 0) {
        human = 1;
    }
    if (du_summary->count > 0) {
        summary = 1;
    }

    /* If no path, use current directory */
    if (du_paths->count == 0) {
        total = du_recursive(".", summary, human);
        if (summary) {
            if (human) {
                format_size(total, size_buf, sizeof(size_buf));
                printf("%s\t.\n", size_buf);
            } else {
                printf("%lld\t.\n", (long long)(total / 1024));
            }
        }
        arg_freetable(du_argtable, 5);
        return EXIT_OK;
    }

    /* Process each path */
    for (i = 0; i < du_paths->count; i++) {
        total = du_recursive(du_paths->filename[i], summary, human);
        if (summary) {
            if (human) {
                format_size(total, size_buf, sizeof(size_buf));
                printf("%s\t%s\n", size_buf, du_paths->filename[i]);
            } else {
                printf("%lld\t%s\n", (long long)(total / 1024), du_paths->filename[i]);
            }
        }
    }

    arg_freetable(du_argtable, 5);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void du_print_usage(FILE *out)
{
    build_du_argtable();

    fprintf(out, "Usage: du ");
    arg_print_syntax(out, du_argtable, "\n");
    fprintf(out, "Summarize disk usage of each FILE, recursively for directories.\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, du_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  du              Show disk usage for current directory\n");
    fprintf(out, "  du -h           Show with human-readable sizes\n");
    fprintf(out, "  du -s /tmp      Show only total for /tmp\n");
    fprintf(out, "  du -sh /tmp     Show total in human-readable format\n");

    arg_freetable(du_argtable, 5);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_du_spec = {
    .name = "du",
    .summary = "estimate file space usage",
    .long_help = "Summarize disk usage of each FILE, recursively for directories.",
    .run = du_run,
    .print_usage = du_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_du_command(void)
{
    register_command(&cmd_du_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_du_spec.run(argc, argv);
}
#endif
