/*
 * pipe_helpers.c - Pipeline execution helpers
 *
 * This file implements pipeline execution using UNIX pipes.
 * Pipes connect stdout of one process to stdin of the next.
 *
 * Example: cat file | grep test | wc -l
 *   - cat's stdout → grep's stdin
 *   - grep's stdout → wc's stdin
 */

#include "picobox.h"
#include "pipe_helpers.h"
#include <sys/wait.h>

/*
 * Execute a pipeline of commands
 *
 * Algorithm:
 * 1. For each command in the pipeline:
 *    - Create a pipe (except for last command)
 *    - Fork a child process
 *    - Child: Connect stdin/stdout to pipes, exec command
 *    - Parent: Close used pipe ends, save pipe for next iteration
 * 2. Close all pipes in parent
 * 3. Wait for all children to complete
 *
 * argv_list: Array of argv arrays (one per command in pipeline)
 * count: Number of commands in pipeline
 *
 * Returns: Exit status of last command in pipeline
 */
int exec_pipeline(char ***argv_list, int count)
{
    int i;
    int status = 0;
    
    /* prev_pipe holds the pipe from the previous iteration
     *  prev_pipe[0] = read end
     *  prev_pipe[1] = write end
     *
     * Initially -1 because there's no "previous" pipe for the first command.
     */
    int prev_pipe[2] = {-1, -1};  

    /* curr_pipe is the pipe we create for the CURRENT command,
     * except for the last command (which writes to stdout).
     */
    int curr_pipe[2];

    /* We'll store ALL children PIDs so the parent can wait for them. */
    pid_t *pids;

    /* Basic argument validation */
    if (count <= 0 || !argv_list) {
        fprintf(stderr, "exec_pipeline: invalid arguments\n");
        return EXIT_ERROR;
    }

    /* Allocate PID array */
    pids = malloc(count * sizeof(pid_t));
    if (!pids) {
        perror("malloc");
        return EXIT_ERROR;
    }

    /* ======================================================
     *  MAIN LOOP: FOR EACH COMMAND IN THE PIPELINE
     *  Example pipeline with 3 commands:
     *      cmd1 | cmd2 | cmd3
     *
     *  This loop runs 3 times.
     * ====================================================== */
    for (i = 0; i < count; i++) {

        /* ------------------------------------------------------
         * If this is NOT the last command, create a pipe.
         * The pipe connects THIS command to the NEXT one.
         *
         * Example for 3 commands:
         *   Iteration 0 → make pipe0
         *   Iteration 1 → make pipe1
         *   Iteration 2 → no pipe (last command)
         * ------------------------------------------------------ */
        if (i < count - 1) {
            if (pipe(curr_pipe) < 0) {
                perror("pipe");
                free(pids);
                return EXIT_ERROR;
            }
        }

        /* Fork a child to run this command */
        pids[i] = fork(); // remeber this returns diffrent kind of pids error 0 or a actual pid 

        if (pids[i] < 0) {
            perror("fork");
            free(pids);
            return EXIT_ERROR;
        }

        /* ========= CHILD PROCESS ========= */
        if (pids[i] == 0) {

            /* -----------------------------------------------------
             * If NOT the first command:
             *   Connect stdin to previous pipe's READ end.
             *
             * Example for "A | B | C":
             *   B gets stdin from pipe0[0]
             *   C gets stdin from pipe1[0]
             * ----------------------------------------------------- */
            if (prev_pipe[0] != -1) {
                if (dup2(prev_pipe[0], STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(EXIT_ERROR);
                }

                /* Child doesn't need the previous pipe fds anymore */
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            /* -----------------------------------------------------
             * If NOT the last command:
             *   Connect stdout to current pipe's WRITE end.
             *
             * Example for "A | B | C":
             *   A → stdout → pipe0[1]
             *   B → stdout → pipe1[1]
             *   C → stdout → normal stdout
             * ----------------------------------------------------- */
            if (i < count - 1) {
                if (dup2(curr_pipe[1], STDOUT_FILENO) < 0) { // simple file descripter redirect
                    perror("dup2");
                    _exit(EXIT_ERROR);
                }

                /* Child doesn't need both ends open */
                close(curr_pipe[0]);
                close(curr_pipe[1]);
            }

            /* -----------------------------------------------------
             * EXECUTE THE COMMAND
             *
             * This replaces the child with the program.
             * If execvp returns, it failed.
             * ----------------------------------------------------- */
            execvp(argv_list[i][0], argv_list[i]); // exec

            perror(argv_list[i][0]);
            _exit(127);   // command not found
        }

        /* ========= PARENT PROCESS ========= */

        /* Close the previous pipe ends —
         * Parent is done with them after forking the child.
         */
        if (prev_pipe[0] != -1) {
            close(prev_pipe[0]);
            close(prev_pipe[1]);
        }

        /* Save current pipe as previous for next loop iteration */
        if (i < count - 1) {
            prev_pipe[0] = curr_pipe[0];  // this pipe’s read side
            prev_pipe[1] = curr_pipe[1];  // this pipe’s write side
        }
    }

    /* After the loop, close any pipe still open */
    if (prev_pipe[0] != -1) {
        close(prev_pipe[0]);
        close(prev_pipe[1]);
    }

    /* ===========================================================
     * WAIT FOR ALL CHILDREN
     * =========================================================== */
    for (i = 0; i < count; i++) {
        int child_status;

        if (waitpid(pids[i], &child_status, 0) < 0) { 
            perror("waitpid");
        } else {
            /* Status of LAST command defines the pipeline's exit code */
            if (i == count - 1) {
                if (WIFEXITED(child_status))
                    status = WEXITSTATUS(child_status);
                else if (WIFSIGNALED(child_status))
                    status = 128 + WTERMSIG(child_status);
            }
        }
    }

    free(pids);

    return status;
}