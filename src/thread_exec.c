#include "thread_exec.h"
#include "picobox.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Thread entry point used for built-in commands in pipelines.
 * Redirects descriptors as needed, runs the built-in, then exits via pthread_exit.
 */
void *thread_execute_builtin(void *arg)
{
    thread_args_t *targs = (thread_args_t *)arg;
    int status = EXIT_ERROR;
    FILE *in = targs->stdin_stream ? targs->stdin_stream : stdin;
    FILE *out = targs->stdout_stream ? targs->stdout_stream : stdout;

    status = targs->spec->run(targs->argc, targs->argv, in, out);

    if (targs->stdout_stream) {
        fflush(targs->stdout_stream);
    } else {
        fflush(stdout);
    }

    thread_args_free(targs);
    pthread_exit((void *)(long)status);
    return NULL;
}

/*
 * Build the thread argument structure, copying argv for exclusive ownership.
 */
thread_args_t *thread_args_create(ExecContext *ctx,
                                  const cmd_spec_t *spec,
                                  int stdin_fd,
                                  int stdout_fd)
{
    thread_args_t *args = calloc(1, sizeof(thread_args_t));
    if (!args) {
        perror("calloc");
        return NULL;
    }

    args->ctx = ctx;
    args->spec = spec;

    args->stdin_stream = NULL;
    args->stdout_stream = NULL;
    args->stdin_owned = false;
    args->stdout_owned = false;

    args->stdin_fd = (stdin_fd != -1) ? dup(stdin_fd) : -1;
    if (stdin_fd != -1 && args->stdin_fd == -1) {
        perror("dup stdin");
        free(args);
        return NULL;
    }
    if (args->stdin_fd != -1) {
        fcntl(args->stdin_fd, F_SETFD, FD_CLOEXEC);
    }

    args->stdout_fd = (stdout_fd != -1) ? dup(stdout_fd) : -1;
    if (stdout_fd != -1 && args->stdout_fd == -1) {
        perror("dup stdout");
        if (args->stdin_fd != -1) {
            close(args->stdin_fd);
        }
        free(args);
        return NULL;
    }
    if (args->stdout_fd != -1) {
        fcntl(args->stdout_fd, F_SETFD, FD_CLOEXEC);
    }

    if (args->stdin_fd != -1) {
        args->stdin_stream = fdopen(args->stdin_fd, "r");
        if (!args->stdin_stream) {
            perror("fdopen stdin");
            close(args->stdin_fd);
            if (args->stdout_fd != -1) {
                close(args->stdout_fd);
            }
            free(args);
            return NULL;
        }
        args->stdin_owned = true;
        args->stdin_fd = -1;
    }

    if (args->stdout_fd != -1) {
        args->stdout_stream = fdopen(args->stdout_fd, "w");
        if (!args->stdout_stream) {
            perror("fdopen stdout");
            close(args->stdout_fd);
            if (args->stdin_owned && args->stdin_stream) {
                fclose(args->stdin_stream);
            }
            free(args);
            return NULL;
        }
        args->stdout_owned = true;
        args->stdout_fd = -1;
    }

    args->argc = ctx->argc;

    args->argv = calloc(ctx->argc + 1, sizeof(char *));
    if (!args->argv) {
        perror("calloc");
        free(args);
        return NULL;
    }

    for (int i = 0; i < ctx->argc; i++) {
        args->argv[i] = strdup(ctx->argv[i]);
        if (!args->argv[i]) {
            perror("strdup");
            thread_args_free(args);
            return NULL;
        }
    }

    return args;
}

/*
 * Release copies allocated for thread execution.
 */
void thread_args_free(thread_args_t *args)
{
    if (!args) {
        return;
    }

    if (args->stdin_owned && args->stdin_stream) {
        fclose(args->stdin_stream);
    }
    if (args->stdout_owned && args->stdout_stream) {
        fclose(args->stdout_stream);
    }

    if (args->argv) {
        for (int i = 0; i < args->argc; i++) {
            free(args->argv[i]);
        }
        free(args->argv);
    }

    free(args);
}
