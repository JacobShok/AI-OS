/*
 * redirect_helpers.h - File redirection helpers
 */

#ifndef REDIRECT_HELPERS_H
#define REDIRECT_HELPERS_H

/* Redirection types */
#define REDIR_INPUT  0   /* < file */
#define REDIR_OUTPUT 1   /* > file */
#define REDIR_APPEND 2   /* >> file */

/* Redirection structure */
struct redirection {
    int type;            /* REDIR_INPUT, REDIR_OUTPUT, or REDIR_APPEND */
    const char *filename; /* File to redirect to/from */
};

/*
 * Apply a single redirection
 * Returns: 0 on success, -1 on error
 */
int apply_redirection(int type, const char *filename);

/*
 * Apply multiple redirections
 * Returns: 0 on success, -1 on error
 */
int apply_redirections(struct redirection *redirections, int count);

#endif /* REDIRECT_HELPERS_H */
