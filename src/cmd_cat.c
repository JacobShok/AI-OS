/*
 * cmd_cat.c - Concatenate files and print to stdout
 *
 * Usage: cat [OPTIONS] [FILE...]
 * Options:
 *   -n    Number all output lines
 */

#include "picobox.h"
#include "utils.h"

static void usage_cat(void)
{
    fprintf(stderr,
        "Usage: cat [OPTION]... [FILE]...\n"
        "Concatenate FILE(s) to standard output.\n"
        "\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -n              number all output lines\n"
        "  -h, --help      display this help and exit\n"
        "\n"
        "Examples:\n"
        "  cat file.txt           Output contents of file.txt\n"
        "  cat file1 file2        Concatenate files and output\n"
        "  cat                    Copy standard input to standard output\n");
}

/*
 * Cat a single file to stdout
 * If filename is "-" or NULL, read from stdin
 * Returns EXIT_OK on success, EXIT_ERROR on failure
 */
static int cat_file(const char *filename, int number_lines, int *line_number)
{
    FILE *fp;
    char buffer[8192];
    int using_stdin = 0;

    /* Determine input source */
    if (filename == NULL || strcmp(filename, "-") == 0) {
        fp = stdin;
        using_stdin = 1;
    } else {
        fp = fopen(filename, "r");
        if (fp == NULL) {
            perror(filename);
            return EXIT_ERROR;
        }
    }

    /* Read and output the file */
    if (number_lines) {
        /* Line-by-line with numbering */
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%6d  %s", (*line_number)++, buffer);
        }
    } else {
        /* Efficient block read */
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {
                perror("cat: write error");
                if (!using_stdin) fclose(fp);
                return EXIT_ERROR;
            }
        }
    }

    /* Check for read errors */
    if (ferror(fp)) {
        perror(filename ? filename : "stdin");
        if (!using_stdin) fclose(fp);
        return EXIT_ERROR;
    }

    /* Close file if it's not stdin */
    if (!using_stdin) {
        fclose(fp);
    }

    return EXIT_OK;
}

int cmd_cat(int argc, char **argv)
{
    int number_lines = 0;
    int line_number = 1;
    int i;
    int first_file_index = 1;
    int ret = EXIT_OK;

    /* Parse options */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage_cat();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-n") == 0) {
            number_lines = 1;
            first_file_index++;
        } else if (argv[i][0] == '-' && argv[i][1] != '\0' && strcmp(argv[i], "-") != 0) {
            fprintf(stderr, "cat: invalid option '%s'\n", argv[i]);
            usage_cat();
            return EXIT_ERROR;
        } else {
            /* First non-option argument */
            break;
        }
    }

    /* If no files specified, read from stdin */
    if (first_file_index >= argc) {
        return cat_file(NULL, number_lines, &line_number);
    }

    /* Process each file calls cat_file for each file*/
    for (i = first_file_index; i < argc; i++) {
        if (cat_file(argv[i], number_lines, &line_number) != EXIT_OK) {
            ret = EXIT_ERROR;
            /* Continue processing remaining files */
        }
    }

    return ret;
}
