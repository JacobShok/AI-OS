/*
 * cmd_grep.c - Search for patterns in files
 */

#include "picobox.h"
#include "utils.h"

static void usage_grep(void)
{
    fprintf(stderr,
        " <pattern> <file>"
        "Usage: grep [OPTION]... PATTERN [FILE]...\n"
        "Search for PATTERN in each FILE.\n"
        "\n"
        "With no FILE, or when FILE is -, read standard input.\n"
        "\n"
        "Options:\n"
        "  -i          ignore case distinctions\n"
        "  -n          print line numbers\n"
        "  -v          invert match (select non-matching lines)\n"
        "  -h, --help  display this help and exit\n");
}

/*
./build/picobox grep <pattern> <file>
*/

static int grep_file(const char *filename, const char *pattern, int ignore_case, int line_numbers, int invert)
{
    FILE *fp;
    char buffer[8192];
    int line_num = 0;
    int found = 0;
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

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        line_num++;
        int match;

        if (ignore_case) {
            match = (strcasestr(buffer, pattern) != NULL);
        } else {
            match = (strstr(buffer, pattern) != NULL);
        }

        if (invert) {
            match = !match;
        }

        if (match) {
            if (line_numbers) {
                printf("%d:", line_num);
            }
            printf("%s", buffer);
            found = 1;
        }
    }

    if (!using_stdin) {
        fclose(fp);
    }

    return found ? EXIT_OK : EXIT_ERROR;
}

int cmd_grep(int argc, char **argv)
{
    int ignore_case = 0;
    int line_numbers = 0;
    int invert = 0;
    int i = 1;
    char *pattern;
    int ret = EXIT_OK;

    /* Parse options */
    while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage_grep();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-i") == 0) {
            ignore_case = 1;
            i++;
        } else if (strcmp(argv[i], "-n") == 0) {
            line_numbers = 1;
            i++;
        } else if (strcmp(argv[i], "-v") == 0) {
            invert = 1;
            i++;
        } else if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else {
            fprintf(stderr, "grep: invalid option '%s'\n", argv[i]);
            usage_grep();
            return EXIT_ERROR;
        }
    }

    /* Need pattern */
    if (i >= argc) {
        fprintf(stderr, "grep: missing pattern\n");
        usage_grep();
        return EXIT_ERROR;
    }

    pattern = argv[i++];

    /* If no files, use stdin */
    if (i >= argc) {
        return grep_file(NULL, pattern, ignore_case, line_numbers, invert);
    }

    /* Process each file */
    for (; i < argc; i++) {
        if (grep_file(argv[i], pattern, ignore_case, line_numbers, invert) != EXIT_OK) {
            ret = EXIT_ERROR;
        }
    }

    return ret;
}
