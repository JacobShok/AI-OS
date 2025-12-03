/*
 * cmd_ls.c - List directory contents (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: ls [OPTIONS] [FILE...]
 * Options:
 *   -a, --all           Do not ignore entries starting with .
 *   -l, --long          Use a long listing format
 *   -h, --human         With -l, print human readable sizes
 *   --help              Display help message
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"
#include "utils.h"

/* Forward declarations */
int ls_run(int argc, char **argv);
void ls_print_usage(FILE *out);
static void print_long_format(const char *path, const char *name, int human);
static int ls_dir(const char *path, int show_all, int long_format, int human);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *ls_help;
static struct arg_lit *ls_all;
static struct arg_lit *ls_long;
static struct arg_lit *ls_human;
static struct arg_file *ls_paths;
static struct arg_end *ls_end;
static void *ls_argtable[7];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_ls_argtable(void)
{
    ls_help = arg_lit0(NULL, "help", "display this help and exit");
    ls_all = arg_lit0("a", "all", "do not ignore entries starting with .");
    ls_long = arg_lit0("l", "long", "use a long listing format");
    ls_human = arg_lit0("h", "human-readable", "with -l, print human readable sizes");
    ls_paths = arg_filen(NULL, NULL, "FILE", 0, 100, "files/directories to list");
    ls_end = arg_end(20);

    ls_argtable[0] = ls_help;
    ls_argtable[1] = ls_all;
    ls_argtable[2] = ls_long;
    ls_argtable[3] = ls_human;
    ls_argtable[4] = ls_paths;
    ls_argtable[5] = ls_end;
    ls_argtable[6] = NULL;
}

/* ===== HELPER FUNCTIONS ===== */

static void print_long_format(const char *path, const char *name, int human)
{
    struct stat st;
    char full_path[4096];
    char time_buf[64];
    char size_buf[32];
    struct passwd *pw;
    struct group *gr;

    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);

    if (lstat(full_path, &st) != 0) {
        perror(name);
        return;
    }

    /* File type and permissions */
    printf("%c", S_ISDIR(st.st_mode) ? 'd' : S_ISLNK(st.st_mode) ? 'l' : '-');
    printf("%c", (st.st_mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (st.st_mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (st.st_mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (st.st_mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (st.st_mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (st.st_mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (st.st_mode & S_IROTH) ? 'r' : '-');
    printf("%c", (st.st_mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (st.st_mode & S_IXOTH) ? 'x' : '-');

    /* Number of links */
    printf(" %3ld", (long)st.st_nlink);

    /* Owner and group */
    pw = getpwuid(st.st_uid);
    gr = getgrgid(st.st_gid);
    printf(" %-8s", pw ? pw->pw_name : "unknown");
    printf(" %-8s", gr ? gr->gr_name : "unknown");

    /* Size */
    if (human) {
        format_size(st.st_size, size_buf, sizeof(size_buf));
        printf(" %8s", size_buf);
    } else {
        printf(" %8ld", (long)st.st_size);
    }

    /* Modification time */
    format_time(st.st_mtime, time_buf, sizeof(time_buf));
    printf(" %s", time_buf);

    /* Name */
    printf(" %s\n", name);
}

static int ls_dir(const char *path, int show_all, int long_format, int human)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        return EXIT_ERROR;
    }

    while ((entry = readdir(dir)) != NULL) {
        /* Skip hidden files unless -a */
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        if (long_format) {
            print_long_format(path, entry->d_name, human);
        } else {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return EXIT_OK;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int ls_run(int argc, char **argv)
{
    int nerrors;
    int show_all = 0;
    int long_format = 0;
    int human = 0;
    int i;
    int ret = EXIT_OK;

    build_ls_argtable();
    nerrors = arg_parse(argc, argv, ls_argtable);

    /* Handle --help */
    if (ls_help->count > 0) {
        ls_print_usage(stdout);
        arg_freetable(ls_argtable, 6);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, ls_end, "ls");
        fprintf(stderr, "Try 'ls --help' for more information.\n");
        arg_freetable(ls_argtable, 6);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    /* Check flags */
    if (ls_all->count > 0) {
        show_all = 1;
    }
    if (ls_long->count > 0) {
        long_format = 1;
    }
    if (ls_human->count > 0) {
        human = 1;
    }

    /* If no directory specified, use current directory */
    if (ls_paths->count == 0) {
        ret = ls_dir(".", show_all, long_format, human);
        arg_freetable(ls_argtable, 6);
        return ret;
    }

    /* List each directory */
    for (i = 0; i < ls_paths->count; i++) {
        if (ls_dir(ls_paths->filename[i], show_all, long_format, human) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    arg_freetable(ls_argtable, 6);
    return ret;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void ls_print_usage(FILE *out)
{
    build_ls_argtable();

    fprintf(out, "Usage: ls ");
    arg_print_syntax(out, ls_argtable, "\n");
    fprintf(out, "List information about the FILEs (the current directory by default).\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, ls_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  ls              List current directory\n");
    fprintf(out, "  ls -a           List all files including hidden\n");
    fprintf(out, "  ls -l           Long format listing\n");
    fprintf(out, "  ls -lh          Long format with human-readable sizes\n");
    fprintf(out, "  ls /tmp         List /tmp directory\n");

    arg_freetable(ls_argtable, 6);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_ls_spec = {
    .name = "ls",
    .summary = "list directory contents",
    .long_help = "List information about the FILEs (the current directory by default).",
    .run = ls_run,
    .print_usage = ls_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_ls_command(void)
{
    register_command(&cmd_ls_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_ls_spec.run(argc, argv);
}
#endif
