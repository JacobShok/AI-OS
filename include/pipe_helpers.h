/*
 * pipe_helpers.h - Pipeline execution helpers
 */

#ifndef PIPE_HELPERS_H
#define PIPE_HELPERS_H

/*
 * Execute a pipeline of commands
 *
 * argv_list: Array of argv arrays (one per command)
 * count: Number of commands in pipeline
 *
 * Returns exit status of last command
 */
int exec_pipeline(char ***argv_list, int count);

#endif /* PIPE_HELPERS_H */
