/*
 * cmd_pwd.c - Print current working directory
 *
 * Usage: pwd [OPTIONS]
 * Options:
 *   -L    Use PWD from environment (logical path with symlinks)
 *   -P    Print physical directory (resolve all symlinks) [default]
 */

#include "picobox.h"
#include "utils.h"
#include <limits.h>

static void usage_pwd(void)
{
    fprintf(stderr,
        "Usage: pwd [OPTION]...\n"
        "Print the full filename of the current working directory.\n"
        "\n"
        "Options:\n"
        "  -L              use PWD from environment, even if it contains symlinks\n"
        "  -P              avoid all symlinks (default)\n"
        "  -h, --help      display this help and exit\n");
}

int cmd_pwd(int argc, char **argv)
{
    int opt;
    int use_logical = 0;
    char cwd[PATH_MAX];
    char *pwd_env;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Handle --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_pwd();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "LPh")) != -1) {
        switch (opt) {
            case 'L':
                use_logical = 1;
                break;
            case 'P':
                use_logical = 0;
                break;
            case 'h':
                usage_pwd();
                return EXIT_OK;
            case '?':
                /* getopt already printed error message */
                usage_pwd();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* If -L, try to use PWD environment variable */
    if (use_logical) {
        pwd_env = getenv("PWD");
        if (pwd_env != NULL) {
            printf("%s\n", pwd_env);
            return EXIT_OK;
        }
        /* Fall through to getcwd() if PWD not set */
    }

    /* Get current working directory using system call */
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd");
        return EXIT_ERROR;
    }

    printf("%s\n", cwd);
    return EXIT_OK;
}
