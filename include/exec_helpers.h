/*
 * exec_helpers.h - Process execution helpers
 */

#ifndef EXEC_HELPERS_H
#define EXEC_HELPERS_H

#include "redirect_helpers.h"

/*
 * Execute command in child process using fork/exec
 * Returns exit status of child process
 */
int exec_command_external(char **argv);

/*
 * Execute command with redirections (Phase 6)
 * Returns exit status of child process
 */
int exec_command_with_redirects(char **argv, struct redirection *redirs, int redir_count);

/*
 * Check if command is a shell built-in
 * Returns 1 if built-in, 0 otherwise
 */
int is_builtin(const char *cmd);

#endif /* EXEC_HELPERS_H */
