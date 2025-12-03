/*
 * exec_helpers.c - Process execution helpers for fork/exec
 *
 * This file implements proper process isolation for command execution.
 * Commands run in separate child processes via fork/exec.
 */

#include "picobox.h"
#include "redirect_helpers.h"
#include <sys/wait.h>

/*
 * Execute command in child process using fork/exec
 *
 * This is the CORRECT way to execute commands in a shell:
 * 1. fork() creates a child process
 * 2. Child calls execvp() to replace itself with the command
 * 3. Parent waits for child to complete
 *
 * Returns: Exit status of child, or EXIT_ERROR on fork/exec error
 */
int exec_command_external(char **argv)
{
    pid_t pid;
    int status;

    if (!argv || !argv[0]) {
        fprintf(stderr, "exec: null command\n");
        return EXIT_ERROR;
    }

    /* Fork a child process */
    pid = fork();

    if (pid < 0) {
        /* Fork failed */
        perror("fork");
        return EXIT_ERROR;
    }

    if (pid == 0) {
        /* === CHILD PROCESS === */

        /* Replace child process with the command */
        execvp(argv[0], argv);

        /* If execvp returns, it failed */
        perror(argv[0]);
        _exit(127);  /* Use _exit() in child, not exit() */
    }

    /* === PARENT PROCESS === */

    /* Wait for child to complete */
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_ERROR;
    }

    /* Check how child exited */
    if (WIFEXITED(status)) {
        /* Child exited normally */
        int exit_code = WEXITSTATUS(status);
        return exit_code;
    }

    if (WIFSIGNALED(status)) {
        /* Child was killed by signal */
        int signal = WTERMSIG(status);
        fprintf(stderr, "%s: terminated by signal %d\n", argv[0], signal);
        return 128 + signal;
    }

    /* Shouldn't get here */
    return EXIT_ERROR;
}

/*
 * Check if command is a shell built-in
 *
 * Built-ins must run in the parent process because they:
 * - Modify shell state (cd changes shell's directory)
 * - Control shell flow (exit terminates shell)
 * - Access shell internals (help shows shell commands)
 */
int is_builtin(const char *cmd)
{
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "help") == 0);
}

/*
 * Execute command with redirections (Phase 6)
 *
 * Redirections are applied in the child process before exec.
 */
int exec_command_with_redirects(char **argv, struct redirection *redirs, int redir_count)
{
    pid_t pid;
    int status;

    if (!argv || !argv[0]) {
        fprintf(stderr, "exec: null command\n");
        return EXIT_ERROR;
    }

    /* Fork a child process */
    pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXIT_ERROR;
    }

    if (pid == 0) {
        /* === CHILD PROCESS === */

        /* Apply redirections */
        if (redir_count > 0 && redirs) {
            if (apply_redirections(redirs, redir_count) < 0) {
                _exit(EXIT_ERROR);
            }
        }

        /* Replace child process with the command */
        execvp(argv[0], argv);

        /* If execvp returns, it failed */
        perror(argv[0]);
        _exit(127);
    }

    /* === PARENT PROCESS === */

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_ERROR;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        fprintf(stderr, "%s: terminated by signal %d\n", argv[0], signal);
        return 128 + signal;
    }

    return EXIT_ERROR;
}
