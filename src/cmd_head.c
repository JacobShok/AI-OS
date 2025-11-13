/*
 * cmd_head.c - Output first N lines of files
 */

#include "picobox.h"
#include "utils.h"

static void usage_head(void)
{
    fprintf(stderr,
        "Usage: head [OPTION]... [FILE]...\n"
        "Print the first 10 lines of each FILE to standard output.\n"
        "With more than one FILE, precede each with a header giving the file name.\n"
        "\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -n NUM      print the first NUM lines instead of the first 10\n"
        "  -h, --help  display this help and exit\n");
}

/* 

./build/picobox head [OPTIONS] <file>

*/

static int head_file(const char *filename, int num_lines)
{
    FILE *fp;
    char buffer[8192];
    int lines = 0;
    int using_stdin = 0;

    if (filename == NULL || strcmp(filename, "-") == 0) {
        fp = stdin;
        using_stdin = 1;
    } else {
        fp = fopen(filename, "r");
        if (!fp) {
            perror(filename);
            return EXIT_ERROR;
        }
    }

    while (lines < num_lines && fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
        lines++;
    }

    if (ferror(fp)) {
        perror(filename ? filename : "stdin");
        if (!using_stdin) fclose(fp);
        return EXIT_ERROR;
    }

    if (!using_stdin) {
        fclose(fp);
    }

    return EXIT_OK;
}


int cmd_head(int argc, char **argv)
{
    int num_lines = 10;
    int i = 1;
    int ret = EXIT_OK;

    /* Parse options */
    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage_head();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-n") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "head: option requires an argument -- 'n'\n");
                return EXIT_ERROR;
            }
            num_lines = atoi(argv[i]);
            if (num_lines < 0) {
                fprintf(stderr, "head: invalid number of lines: '%s'\n", argv[i]);
                return EXIT_ERROR;
            }
            i++;
        } else if (strcmp(argv[i], "-") == 0) {
            break;
        } else if (str_starts_with(argv[i], "-n")) {
            num_lines = atoi(argv[i] + 2);
            if (num_lines < 0) {
                fprintf(stderr, "head: invalid number of lines: '%s'\n", argv[i] + 2);
                return EXIT_ERROR;
            }
            i++;
        } else {
            fprintf(stderr, "head: invalid option '%s'\n", argv[i]);
            usage_head();
            return EXIT_ERROR;
        }
    }

    /* If no files, use stdin */
    if (i >= argc) {
        return head_file(NULL, num_lines);
    }

    /* Process each file */
    for (; i < argc; i++) {
        if (head_file(argv[i], num_lines) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    return ret;
}
