/*
 * cmd_ls.c - List directory contents
 */

#include "picobox.h"
#include "utils.h"
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

static void usage_ls(void)
{
    fprintf(stderr,
        "Usage: ls [OPTION]... [FILE]...\n"
        "List information about the FILEs (the current directory by default).\n"
        "\n"
        "Options:\n"
        "  -a          do not ignore entries starting with .\n"
        "  -l          use a long listing format\n"
        "  -h          with -l, print human readable sizes\n"
        "  -h, --help  display this help and exit\n");
}

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

// 
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

int cmd_ls(int argc, char **argv)
{
    int show_all = 0;
    int long_format = 0;
    int human = 0;
    int i = 1;
    int ret = EXIT_OK;

    /* Parse options */
    // loops every parameter to set flags, skips the file path param if there is one becuase it does not have -
    // , always looks for --help first
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "--help") == 0) {
            usage_ls();
            return EXIT_OK;
        }

        /* Handle combined flags like -lah */
        for (int j = 1; argv[i][j]; j++) {
            switch (argv[i][j]) {
                case 'a':
                    show_all = 1;
                    break;
                case 'l':
                    long_format = 1;
                    break;
                case 'h':
                    human = 1;
                    break;
                default:
                    fprintf(stderr, "ls: invalid option '-%c'\n", argv[i][j]);
                    usage_ls();
                    return EXIT_ERROR;
            }
        }
        i++;
    }


    /* If no directory specified, use current directory */
    if (i >= argc) {
        return ls_dir(".", show_all, long_format, human);
    }

    /* List each directory if a file was specified loops through the rest of argv param until argc */
    for (; i < argc; i++) {
        if (ls_dir(argv[i], show_all, long_format, human) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    // always returns if something went wrong ex. 3 file paths were listed the first failed the next 2 worked
    // sill a error message is returned
    return ret;
}
