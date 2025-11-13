/*
 * cmd_cp.c - Copy files and directories
 */

#include "picobox.h"
#include "utils.h"
#include <dirent.h>

static void usage_cp(void)
{
    fprintf(stderr,
        "Usage: cp [OPTION]... SOURCE DEST\n"
        "Copy SOURCE to DEST.\n"
        "\n"
        "Options:\n"
        "  -r, -R      copy directories recursively\n"
        "  -f          force overwrite\n"
        "  -h, --help  display this help and exit\n");
}

/*
cp file1.txt file2.txt              # Copy single file
cp script.sh /usr/local/bin/        # Copy file to directory
cp -r mydir backup/                 # Copy directory recursively
cp -r /home/user/docs /backup/      # Recursive copy with full paths*/

static int cp_recursive(const char *src, const char *dest);

static int cp_file_to_file(const char *src, const char *dest)
{
    if (copy_file(src, dest) < 0) {
        perror(dest);
        return EXIT_ERROR;
    }
    return EXIT_OK;
}

// for copying directories
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

int cmd_cp(int argc, char **argv)
{
    int recursive = 0;
    int i = 1;
    char *src, *dest;

    /* Parse options */
    /*
    -r / -R: Enable recursive copying (for directories)
    -f: Force flag (accepted but not implemented - just skipped)
    --: Stop option parsing (allows filenames starting with -) */
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage_cp();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) {
            recursive = 1;
            i++;
        } else if (strcmp(argv[i], "-f") == 0) {
            i++;
        } else if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else {
            fprintf(stderr, "cp: invalid option '%s'\n", argv[i]);
            usage_cp();
            return EXIT_ERROR;
        }
    }

    /* Need source and dest */
    if (i + 2 != argc) {
        fprintf(stderr, "cp: missing file operand\n");
        usage_cp();
        return EXIT_ERROR;
    }

    src = argv[i];
    dest = argv[i + 1];

    /* If source is a directory but -r wasn't specified → error */
    if (is_directory(src) && !recursive) {
        fprintf(stderr, "cp: '%s' is a directory (use -r)\n", src);
        return EXIT_ERROR;
    }

    if (recursive) {
        return cp_recursive(src, dest);
    } else {
        return cp_file_to_file(src, dest);
    }
}
