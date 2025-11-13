/*
 * cmd_ln.c - Create links
 */

#include "picobox.h"
#include "utils.h"

static void usage_ln(void)
{
    fprintf(stderr,
        "Usage: ln [OPTION]... TARGET LINK_NAME\n"
        "Create a link to TARGET with the name LINK_NAME.\n"
        "\n"
        "Options:\n"
        "  -s          make symbolic links instead of hard links\n"
        "  -f          remove existing destination files\n"
        "  -h, --help  display this help and exit\n");
}

int cmd_ln(int argc, char **argv)
{
    int symbolic = 0;
    int force = 0;
    int i = 1;
    char *target, *linkname;

    /* Parse options */
    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage_ln();
            return EXIT_OK;
        } else if (strcmp(argv[i], "-s") == 0) {
            symbolic = 1;
            i++;
        } else if (strcmp(argv[i], "-f") == 0) {
            force = 1;
            i++;
        } else if (strcmp(argv[i], "--") == 0) {
            i++;
            break;
        } else {
            fprintf(stderr, "ln: invalid option '%s'\n", argv[i]);
            usage_ln();
            return EXIT_ERROR;
        }
    }

    /* Need two arguments: target and linkname */
    if (i + 2 != argc) {
        fprintf(stderr, "ln: missing operand\n");
        usage_ln();
        return EXIT_ERROR;
    }

    target = argv[i];
    linkname = argv[i + 1];

    /* Remove existing link if -f */
    if (force) {
        unlink(linkname);
    }

    /* Create link */
    if (symbolic) {
        if (symlink(target, linkname) != 0) {
            perror("ln");
            return EXIT_ERROR;
        }
    } else {
        if (link(target, linkname) != 0) {
            perror("ln");
            return EXIT_ERROR;
        }
    }

    return EXIT_OK;
}
