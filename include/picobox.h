#ifndef PICOBOX_H
#define PICOBOX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Version information */
#define PICOBOX_VERSION "0.1.0"

/* Return codes */
#define EXIT_OK 0
#define EXIT_ERROR 1

/* Command function type - all commands follow this signature */
typedef int (*cmd_func_t)(int argc, char **argv);

/* Command structure for dispatcher */
struct command {
    const char *name;
    cmd_func_t func;
};

/* Command function prototypes */
/* These will be implemented in separate cmd_*.c files */

/* Day 2-3: First 5 commands */
int cmd_echo(int argc, char **argv);
int cmd_pwd(int argc, char **argv);
int cmd_cat(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_touch(int argc, char **argv);

/* Day 4-5: File operations */
int cmd_ls(int argc, char **argv);
int cmd_cp(int argc, char **argv);
int cmd_rm(int argc, char **argv);
int cmd_mv(int argc, char **argv);

/* Day 6-7: Text processing */
int cmd_head(int argc, char **argv);
int cmd_tail(int argc, char **argv);
int cmd_wc(int argc, char **argv);
int cmd_ln(int argc, char **argv);

/* Day 8-9: Search utilities */
int cmd_grep(int argc, char **argv);
int cmd_find(int argc, char **argv);
int cmd_basename(int argc, char **argv);
int cmd_dirname(int argc, char **argv);

/* Day 10-11: File permissions & system info */
int cmd_chmod(int argc, char **argv);
int cmd_stat(int argc, char **argv);
int cmd_du(int argc, char **argv);
int cmd_df(int argc, char **argv);

/* Day 12-13: Process & environment */
int cmd_env(int argc, char **argv);
int cmd_sleep(int argc, char **argv);
int cmd_true(int argc, char **argv);
int cmd_false(int argc, char **argv);

/* Shell mode */
int shell_main(void);

#endif /* PICOBOX_H */
