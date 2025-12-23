#ifndef THREAD_EXEC_H
#define THREAD_EXEC_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "Skeleton.h"
#include "cmd_spec.h"

/*
 * Argument bundle that each built-in thread receives.
 * Note argv is a private copy while ctx is shared.
 */
typedef struct thread_args {
    ExecContext *ctx;          /* Shared execution context */
    int argc;                  /* Argument count */
    char **argv;               /* Thread-private argv copy */
    int stdin_fd;              /* Input descriptor for this stage */
    int stdout_fd;             /* Output descriptor for this stage */
    FILE *stdin_stream;        /* Stream wrapping stdin_fd (or NULL for stdin) */
    FILE *stdout_stream;       /* Stream wrapping stdout_fd (or NULL for stdout) */
    bool stdin_owned;          /* Should stdin_stream be fclose'd */
    bool stdout_owned;         /* Should stdout_stream be fclose'd */
    const cmd_spec_t *spec;    /* Built-in command spec */
} thread_args_t;

void *thread_execute_builtin(void *arg);
thread_args_t *thread_args_create(ExecContext *ctx,
                                  const cmd_spec_t *spec,
                                  int stdin_fd,
                                  int stdout_fd);
void thread_args_free(thread_args_t *args);

#endif /* THREAD_EXEC_H */
