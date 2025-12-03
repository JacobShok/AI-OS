/*
 * redirect_helpers.c - File redirection helpers
 *
 * This file implements input/output redirection for commands.
 * Redirections modify stdin/stdout/stderr before executing commands.
 *
 * Supported redirections:
 *   < file   - Read stdin from file
 *   > file   - Write stdout to file (truncate)
 *   >> file  - Append stdout to file
 */

#include "picobox.h"
#include "redirect_helpers.h"
#include <fcntl.h>

/*
 * Apply a single redirection
 *
 * Returns: 0 on success, -1 on error
 */
int apply_redirection(int type, const char *filename)
{
    int fd;
    int target_fd;

    if (!filename) {
        fprintf(stderr, "Error: NULL filename for redirection\n");
        return -1;
    }

    switch (type) {
        case REDIR_INPUT:
            /* Open file for reading, redirect to stdin */
            fd = open(filename, O_RDONLY);
            if (fd < 0) {
                perror(filename);
                return -1;
            }
            target_fd = STDIN_FILENO;
            break;

        case REDIR_OUTPUT:
            /* Open file for writing (truncate), redirect to stdout */
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror(filename);
                return -1;
            }
            target_fd = STDOUT_FILENO;
            break;

        case REDIR_APPEND:
            /* Open file for appending, redirect to stdout */
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror(filename);
                return -1;
            }
            target_fd = STDOUT_FILENO;
            break;

        default:
            fprintf(stderr, "Error: Unknown redirection type %d\n", type);
            return -1;
    }

    /* Duplicate fd to target (stdin/stdout) */
    if (dup2(fd, target_fd) < 0) {
        perror("dup2");
        close(fd);
        return -1;
    }

    /* Close original fd (we've duplicated it) */
    close(fd);

    return 0;
}

/*
 * Apply multiple redirections
 *
 * redirections: Array of redirection structures
 * count: Number of redirections
 *
 * Returns: 0 on success, -1 on error
 */
int apply_redirections(struct redirection *redirections, int count)
{
    int i;


    if (!redirections && count > 0) {
        fprintf(stderr, "Error: NULL redirections array\n");
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (apply_redirection(redirections[i].type, redirections[i].filename) < 0) {
            return -1;
        }
    }

    return 0;
}
