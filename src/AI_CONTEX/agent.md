agent.md
Agent Instructions: Shell Variables & Thread-Based Execution
This document provides step-by-step instructions for implementing the plan.md. Follow these instructions sequentially and test after each step.

AGENT WORKFLOW
General Rules:

Read before coding: Understand what you're implementing
Test after every change: Don't continue if tests fail
Commit frequently: Small, logical commits with clear messages
Ask if stuck: Don't waste time on blocked tasks


PHASE 1: SHELL VARIABLES
Step 1.1: Create Hash Table Implementation
Task: Create include/var_table.h and src/var_table.c
File: include/var_table.h
c#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include <stddef.h>

/* Variable table entry (hash bucket node) */
typedef struct var_entry {
    char *name;
    char *value;
    struct var_entry *next;
} var_entry_t;

/* Variable table (hash table) */
typedef struct var_table {
    var_entry_t **buckets;
    size_t size;
    size_t count;
} var_table_t;

/* Create new variable table with given bucket count */
var_table_t* var_table_create(size_t size);

/* Destroy variable table and free all memory */
void var_table_destroy(var_table_t *table);

/* Set variable (creates if doesn't exist, updates if exists) */
int var_table_set(var_table_t *table, const char *name, const char *value);

/* Get variable value (returns NULL if not found) */
const char* var_table_get(var_table_t *table, const char *name);

/* Unset (delete) variable (returns 0 on success, -1 if not found) */
int var_table_unset(var_table_t *table, const char *name);

#endif /* VAR_TABLE_H */
File: src/var_table.c
c#include "var_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Simple hash function (djb2 algorithm)
 * Converts string to bucket index
 */
static unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    
    return hash;
}

/*
 * Create new variable table
 */
var_table_t* var_table_create(size_t size)
{
    var_table_t *table = malloc(sizeof(var_table_t));
    if (!table) {
        perror("malloc");
        return NULL;
    }
    
    table->size = size;
    table->count = 0;
    
    /* Allocate bucket array and initialize to NULL */
    table->buckets = calloc(size, sizeof(var_entry_t *));
    if (!table->buckets) {
        perror("calloc");
        free(table);
        return NULL;
    }
    
    return table;
}

/*
 * Destroy variable table
 * Frees all entries and the table itself
 */
void var_table_destroy(var_table_t *table)
{
    if (!table) return;
    
    /* Free all entries in all buckets */
    for (size_t i = 0; i < table->size; i++) {
        var_entry_t *entry = table->buckets[i];
        while (entry) {
            var_entry_t *next = entry->next;
            free(entry->name);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }
    
    free(table->buckets);
    free(table);
}

/*
 * Set variable in table
 * Creates new entry or updates existing one
 * Returns 0 on success, -1 on error
 */
int var_table_set(var_table_t *table, const char *name, const char *value)
{
    if (!table || !name || !value) {
        return -1;
    }
    
    /* Calculate bucket index */
    unsigned long h = hash(name);
    size_t index = h % table->size;
    
    /* Search for existing entry */
    var_entry_t *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            /* Found existing entry - update value */
            char *new_value = strdup(value);
            if (!new_value) {
                perror("strdup");
                return -1;
            }
            free(entry->value);
            entry->value = new_value;
            return 0;
        }
        entry = entry->next;
    }
    
    /* Not found - create new entry */
    entry = malloc(sizeof(var_entry_t));
    if (!entry) {
        perror("malloc");
        return -1;
    }
    
    entry->name = strdup(name);
    entry->value = strdup(value);
    
    if (!entry->name || !entry->value) {
        perror("strdup");
        free(entry->name);
        free(entry->value);
        free(entry);
        return -1;
    }
    
    /* Insert at head of bucket list */
    entry->next = table->buckets[index];
    table->buckets[index] = entry;
    table->count++;
    
    return 0;
}

/*
 * Get variable from table
 * Returns value or NULL if not found
 */
const char* var_table_get(var_table_t *table, const char *name)
{
    if (!table || !name) {
        return NULL;
    }
    
    /* Calculate bucket index */
    unsigned long h = hash(name);
    size_t index = h % table->size;
    
    /* Search for entry */
    var_entry_t *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    return NULL;  /* Not found */
}

/*
 * Unset (delete) variable from table
 * Returns 0 on success, -1 if not found
 */
int var_table_unset(var_table_t *table, const char *name)
{
    if (!table || !name) {
        return -1;
    }
    
    /* Calculate bucket index */
    unsigned long h = hash(name);
    size_t index = h % table->size;
    
    /* Search for entry */
    var_entry_t **ptr = &table->buckets[index];
    while (*ptr) {
        var_entry_t *entry = *ptr;
        if (strcmp(entry->name, name) == 0) {
            /* Found it - remove from list */
            *ptr = entry->next;
            free(entry->name);
            free(entry->value);
            free(entry);
            table->count--;
            return 0;
        }
        ptr = &entry->next;
    }
    
    return -1;  /* Not found */
}
Compile & Test:
bash# Add to Makefile:
# SRCS += src/var_table.c

make clean && make

# Create test program: tests/test_var_table.c
# Compile and run basic tests
./test_var_table
Test Checklist:

 Create table succeeds
 Set and get variable works
 Update existing variable works
 Get non-existent returns NULL
 Unset works
 No memory leaks (valgrind)


Step 1.2: Integrate Variable Table into ExecContext
Task: Modify include/Skeleton.h and src/bnfc/Skeleton.c
File: include/Skeleton.h (add near top):
c#include "var_table.h"  /* Add this include */
File: include/Skeleton.h (modify struct):
ctypedef struct exec_context {
    /* ... existing fields ... */
    
    /* Variable storage */
    var_table_t *shell_vars;   /* Shell-local variables */
    
} ExecContext;
File: src/bnfc/Skeleton.c (modify exec_context_new):
cExecContext *exec_context_new(void)
{
    ExecContext *ctx = calloc(1, sizeof(ExecContext));
    if (!ctx) {
        perror("calloc");
        return NULL;
    }

    /* ... existing initialization ... */
    
    /* Initialize variable table (128 buckets) */
    ctx->shell_vars = var_table_create(128);
    if (!ctx->shell_vars) {
        fprintf(stderr, "Failed to create variable table\n");
        free(ctx->argv);
        free(ctx->pids);
        free(ctx);
        return NULL;
    }

    return ctx;
}
File: src/bnfc/Skeleton.c (modify exec_context_free):
cvoid exec_context_free(ExecContext *ctx)
{
    if (!ctx) return;

    /* ... existing cleanup ... */
    
    /* Free variable table */
    var_table_destroy(ctx->shell_vars);
    
    free(ctx);
}
Compile & Test:
bashmake clean && make
./picobox
# Should start without errors
exit
Test Checklist:

 Shell starts without errors
 Shell exits cleanly
 No memory leaks (valgrind ./picobox)


Step 1.3: Implement Variable Expansion
Task: Modify visitWord() in src/bnfc/Skeleton.c
Add helper function BEFORE visitWord():
c/*
 * Expand variable references in a word
 * 
 * Supports:
 *   $VAR       - Shell or environment variable
 *   $$         - Process ID
 *   $?         - Last exit status
 *   $0         - Shell name
 * 
 * Returns: Newly allocated string with expansions (caller must free)
 */
static char* expand_variables(const char *word, ExecContext *ctx)
{
    /* Allocate result buffer (max 4KB) */
    char *result = malloc(4096);
    if (!result) {
        perror("malloc");
        return NULL;
    }
    
    char *dst = result;
    const char *src = word;
    
    while (*src && (dst - result) < 4000) {  /* Leave room for null */
        if (*src == '$') {
            src++;  /* Skip $ */
            
            /* Handle special variables */
            if (*src == '$') {
                /* $$ = process ID */
                dst += sprintf(dst, "%d", getpid());
                src++;
                continue;
            } else if (*src == '?') {
                /* $? = last exit status */
                dst += sprintf(dst, "%d", ctx->exit_status);
                src++;
                continue;
            } else if (*src == '0') {
                /* $0 = shell name */
                dst += sprintf(dst, "picobox");
                src++;
                continue;
            } else if (!isalnum(*src) && *src != '_') {
                /* Just a $ with no valid var name - keep literal */
                *dst++ = '$';
                continue;
            }
            
            /* Extract variable name (alphanumeric + underscore) */
            char varname[256];
            int i = 0;
            while (*src && (isalnum(*src) || *src == '_') && i < 255) {
                varname[i++] = *src++;
            }
            varname[i] = '\0';
            
            if (i == 0) {
                /* Shouldn't happen, but just in case */
                *dst++ = '$';
                continue;
            }
            
            /* Look up variable - shell vars first, then environment */
            const char *value = var_table_get(ctx->shell_vars, varname);
            if (!value) {
                value = getenv(varname);
            }
            
            if (value) {
                /* Copy value into result */
                size_t value_len = strlen(value);
                if ((dst - result) + value_len < 4000) {
                    strcpy(dst, value);
                    dst += value_len;
                }
            }
            /* If not found, expand to empty string (POSIX behavior) */
            
        } else {
            /* Regular character - copy as-is */
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    return result;
}
Modify visitWord():
cvoid visitWord(Word p, ExecContext *ctx)
{
    /* Grow argv if needed */
    if (ctx->argc >= ctx->argv_capacity - 1) {
        ctx->argv_capacity *= 2;
        ctx->argv = realloc(ctx->argv, ctx->argv_capacity * sizeof(char *));
        if (!ctx->argv) {
            perror("realloc");
            exit(1);
        }
    }

    /* Expand variables in word, then add to argv */
    ctx->argv[ctx->argc] = expand_variables(p, ctx);
    if (!ctx->argv[ctx->argc]) {
        perror("expand_variables");
        exit(1);
    }
    ctx->argc++;
}
Compile & Test:
bashmake clean && make
./picobox
$ echo $$
<should print PID>
$ echo $HOME
<should print home directory>
$ echo $NOTEXIST
<should print empty line>
$ exit
Test Checklist:

 echo $$ prints process ID
 echo $HOME prints home directory
 echo $NOTEXIST prints empty line
 echo $? prints 0 (last exit status)
 No memory leaks


Step 1.4: Add Variable Assignment
Task: Modify visitSimpleCommand() to detect VAR=VALUE pattern
File: src/bnfc/Skeleton.c (in visitSimpleCommand, after building argv):
Find this section:
c/* Skip if no command */
if (ctx->argc == 0 || !ctx->argv[0]) {
    break;
}
Add BEFORE the execution logic (before if (ctx->in_pipeline)):
c/* Check for variable assignment (VAR=VALUE) */
if (strchr(ctx->argv[0], '=') != NULL) {
    /* Might be an assignment - validate it's not a command with = in path */
    if (strchr(ctx->argv[0], '/') == NULL) {
        /* No slash - likely an assignment */
        char *equals = strchr(ctx->argv[0], '=');
        *equals = '\0';  /* Split at = */
        char *name = ctx->argv[0];
        char *value = equals + 1;
        
        /* Validate variable name (must start with letter or underscore) */
        if (isalpha(name[0]) || name[0] == '_') {
            /* Valid variable name - set it */
            if (var_table_set(ctx->shell_vars, name, value) == 0) {
                ctx->exit_status = EXIT_OK;
            } else {
                fprintf(stderr, "Failed to set variable %s\n", name);
                ctx->exit_status = EXIT_ERROR;
            }
            break;  /* Don't execute as command */
        } else {
            /* Invalid variable name - restore = and try to execute */
            *equals = '=';
        }
    }
}
Compile & Test:
bashmake clean && make
./picobox
$ myvar=hello
$ echo $myvar
hello
$ count=42
$ echo $count
42
$ 123bad=value
<should give error or execute as command>
$ exit
Test Checklist:

 myvar=hello works
 echo $myvar prints "hello"
 Reassignment works (myvar=world; echo $myvar → "world")
 Invalid names rejected (123var=test)
 No memory leaks


Step 1.5: Add Export Built-in
Task: Add export command to set environment variables
File: src/bnfc/Skeleton.c (add helper function):
c/*
 * export built-in command
 * 
 * Usage:
 *   export VAR=VALUE    - Set and export to environment
 *   export VAR          - Export existing shell variable
 */
static int builtin_export(ExecContext *ctx)
{
    if (ctx->argc < 2) {
        fprintf(stderr, "export: usage: export VAR=VALUE or export VAR\n");
        return EXIT_ERROR;
    }
    
    for (int i = 1; i < ctx->argc; i++) {
        char *arg = ctx->argv[i];
        char *equals = strchr(arg, '=');
        
        if (equals) {
            /* export VAR=VALUE */
            *equals = '\0';
            char *name = arg;
            char *value = equals + 1;
            
            /* Validate variable name */
            if (!isalpha(name[0]) && name[0] != '_') {
                fprintf(stderr, "export: invalid variable name: %s\n", name);
                *equals = '=';  /* Restore for error message */
                return EXIT_ERROR;
            }
            
            if (setenv(name, value, 1) != 0) {
                perror("export: setenv");
                return EXIT_ERROR;
            }
        } else {
            /* export VAR - export existing shell variable */
            const char *value = var_table_get(ctx->shell_vars, arg);
            if (!value) {
                fprintf(stderr, "export: %s: not found\n", arg);
                return EXIT_ERROR;
            }
            
            if (setenv(arg, value, 1) != 0) {
                perror("export: setenv");
                return EXIT_ERROR;
            }
        }
    }
    
    return EXIT_OK;
}
File: src/bnfc/Skeleton.c (add to built-in checks in visitSimpleCommand):
Find this section:
c/* Check for built-ins that MUST run in parent */
if (strcmp(ctx->argv[0], "exit") == 0) {
    ctx->should_exit = 1;
    ctx->exit_status = EXIT_OK;
    break;
} else if (strcmp(ctx->argv[0], "cd") == 0) {
    ctx->exit_status = builtin_cd(ctx);
    break;
} else if (strcmp(ctx->argv[0], "help") == 0) {
    ctx->exit_status = builtin_help(ctx);
    break;
}
Add after the help check:
c} else if (strcmp(ctx->argv[0], "export") == 0) {
    ctx->exit_status = builtin_export(ctx);
    break;
}
Compile & Test:
bashmake clean && make
./picobox
$ myvar=test
$ export myvar
$ echo $myvar
test
$ export NEWVAR=hello
$ echo $NEWVAR
hello
$ exit
Test Checklist:

 export VAR=VALUE sets environment variable
 export VAR exports shell variable to environment
 Exported variables visible in child processes
 No memory leaks


PHASE 2: THREAD-BASED PIPELINE EXECUTION
Step 2.1: Add Thread Support to ExecContext
Task: Modify include/Skeleton.h to track threads
File: include/Skeleton.h (add include at top):
c#include <pthread.h>
File: include/Skeleton.h (modify struct):
ctypedef struct exec_context {
    /* ... existing fields ... */
    
    /* Thread tracking (for built-in commands in pipelines) */
    pthread_t *threads;      /* Array of thread IDs */
    int thread_count;        /* Number of threads created */
    int thread_capacity;     /* Allocated capacity */
    
} ExecContext;
File: src/bnfc/Skeleton.c (modify exec_context_new):
c/* Initialize thread tracking array */
ctx->thread_capacity = 8;
ctx->threads = malloc(ctx->thread_capacity * sizeof(pthread_t));
if (!ctx->threads) {
    var_table_destroy(ctx->shell_vars);
    free(ctx->pids);
    free(ctx->argv);
    free(ctx);
    return NULL;
}
ctx->thread_count = 0;
File: src/bnfc/Skeleton.c (modify exec_context_free):
c/* Free thread array */
free(ctx->threads);
Compile & Test:
bashmake clean && make
./picobox
$ echo test
test
$ exit
Test Checklist:

 Shell compiles with pthread
 Shell starts without errors
 No memory leaks


Step 2.2: Create Thread Execution Infrastructure
Task: Create include/thread_exec.h and src/thread_exec.c
File: include/thread_exec.h:
c#ifndef THREAD_EXEC_H
#define THREAD_EXEC_H

#include "Skeleton.h"
#include "cmd_spec.h"
#include <pthread.h>

/*
 * Thread argument structure
 * Contains everything a thread needs to execute a built-in command
 * 
 * IMPORTANT: argv is COPIED (thread-private) but ctx is SHARED
 */
typedef struct thread_args {
    int argc;                /* Argument count */
    char **argv;             /* Argument array (THREAD-PRIVATE COPY) */
    int stdin_fd;            /* File descriptor for stdin (-1 if none) */
    int stdout_fd;           /* File descriptor for stdout (-1 if none) */
    const cmd_spec_t *spec;  /* Command specification (function to call) */
    int exit_status;         /* Thread's exit status (set before exit) */
} thread_args_t;

/*
 * Thread entry point for built-in command execution
 * This is what pthread_create() calls
 * 
 * @param arg - Pointer to thread_args_t structure
 * @return Exit status (as void pointer)
 */
void* thread_execute_builtin(void *arg);

/*
 * Create thread args structure
 * Makes copies of argc/argv so thread has private data
 * 
 * @param argc - Argument count
 * @param argv - Argument array (will be COPIED)
 * @param stdin_fd - stdin file descriptor
 * @param stdout_fd - stdout file descriptor
 * @param spec - Command spec
 * @return Allocated thread_args_t (caller must NOT free - thread will free)
 */
thread_args_t* thread_args_create(
    int argc,
    char **argv,
    int stdin_fd,
    int stdout_fd,
    const cmd_spec_t *spec
);

/*
 * Free thread args structure
 * Called by thread before exiting
 * 
 * @param args - Thread args to free
 */
void thread_args_free(thread_args_t *args);

#endif /* THREAD_EXEC_H */
File: src/thread_exec.c:
c#include "thread_exec.h"
#include "picobox.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Thread entry point for built-in command execution
 * 
 * This function runs in a SEPARATE THREAD within the SAME PROCESS.
 * Key differences from fork/exec:
 * - Shares address space with parent (same memory)
 * - Shares file descriptors (pipes work!)
 * - Has own stack
 * - MUST use pthread_exit(), NOT exit()
 * 
 * Algorithm:
 * 1. Redirect stdin/stdout using dup2()
 * 2. Call built-in command function directly
 * 3. Flush output buffers
 * 4. Free thread-private memory
 * 5. Exit thread with status
 */
void* thread_execute_builtin(void *arg)
{
    thread_args_t *targs = (thread_args_t *)arg;
    int status;
    
    /* Redirect stdin if needed */
    if (targs->stdin_fd != -1 && targs->stdin_fd != STDIN_FILENO) {
        if (dup2(targs->stdin_fd, STDIN_FILENO) < 0) {
            perror("thread: dup2 stdin");
            status = EXIT_ERROR;
            goto cleanup;
        }
        close(targs->stdin_fd);
    }
    
    /* Redirect stdout if needed */
    if (targs->stdout_fd != -1 && targs->stdout_fd != STDOUT_FILENO) {
        if (dup2(targs->stdout_fd, STDOUT_FILENO) < 0) {
            perror("thread: dup2 stdout");
            status = EXIT_ERROR;
            goto cleanup;
        }
        close(targs->stdout_fd);
    }
    
    /* Execute the built-in command function directly */
    status = targs->spec->run(targs->argc, targs->argv);
    
    /* Flush output streams (CRITICAL for threads!) */
    fflush(stdout);
    fflush(stderr);
    
cleanup:
    /* Free thread-private memory */
    thread_args_free(targs);
    
    /* Exit thread with status (pthread_join can retrieve this) */
    pthread_exit((void *)(long)status);
    
    return NULL;  /* Never reached, but makes compiler happy */
}

/*
 * Create thread args structure
 * Makes COPIES of argv so thread has private data
 */
thread_args_t* thread_args_create(
    int argc,
    char **argv,
    int stdin_fd,
    int stdout_fd,
    const cmd_spec_t *spec)
{
    thread_args_t *args = malloc(sizeof(thread_args_t));
    if (!args) {
        perror("malloc");
        return NULL;
    }
    
    args->argc = argc;
    args->spec = spec;
    args->stdin_fd = stdin_fd;
    args->stdout_fd = stdout_fd;
    args->exit_status = 0;
    
    /* Allocate argv array */
    args->argv = malloc((argc + 1) * sizeof(char *));
    if (!args->argv) {
        perror("malloc");
        free(args);
        return NULL;
    }
    
    /* Copy each argument string */
    for (int i = 0; i < argc; i++) {
        args->argv[i] = strdup(argv[i]);
        if (!args->argv[i]) {
            perror("strdup");
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(args->argv[j]);
            }
            free(args->argv);
            free(args);
            return NULL;
        }
    }
    args->argv[argc] = NULL;
    
    return args;
}

/*
 * Free thread args structure
 */
void thread_args_free(thread_args_t *args)
{
    if (!args) return;
    
    /* Free argv copies */
    if (args->argv) {
        for (int i = 0; i < args->argc; i++) {
            free(args->argv[i]);
        }
        free(args->argv);
    }
    
    free(args);
}
Update Makefile:
makefile# Add to SRCS
SRCS += src/thread_exec.c

# Add pthread flag
LDFLAGS += -pthread
Compile & Test:
bashmake clean && make
# Should compile successfully
Test Checklist:

 Compiles without errors
 Compiles without warnings
 Links successfully (pthread library)


Step 2.3: Modify visitPipeline() for Thread Support
Task: Update visitPipeline() to use threads for built-in commands
IMPORTANT: This is the most complex change. Read the entire modification carefully before implementing.
File: src/bnfc/Skeleton.c (add include at top):
c#include "thread_exec.h"
File: src/bnfc/Skeleton.c (REPLACE entire visitPipeline function):
c/*
 * Visit Pipeline - THREAD-AWARE VERSION
 *
 * This function creates a pipeline of commands.
 * For BUILT-IN commands: Uses pthreads
 * For EXTERNAL commands: Uses fork/exec
 * 
 * Pipes connect commands regardless of thread vs process.
 * 
 * Algorithm:
 * 1. Count commands in pipeline
 * 2. For each command:
 *    a. Create pipe (except for last)
 *    b. Build argv for this command
 *    c. Detect if built-in or external
 *    d. If built-in: pthread_create()
 *    e. If external: fork/exec
 * 3. Close all pipes in parent
 * 4. Wait for all threads (pthread_join)
 * 5. Wait for all processes (waitpid)
 */
void visitPipeline(Pipeline p, ExecContext *ctx)
{
    switch(p->kind)
    {
    case is_PipeLine:
    {
        /* Count commands in pipeline */
        int cmd_count = 0;
        ListSimpleCommand temp = p->u.pipeLine_.listsimplecommand_;
        while (temp) {
            cmd_count++;
            temp = temp->listsimplecommand_;
        }

        /* Set pipeline context */
        ctx->in_pipeline = 1;
        ctx->pipeline_total = cmd_count;
        ctx->prev_pipe[0] = -1;
        ctx->prev_pipe[1] = -1;

        /* Ensure PIDs and threads arrays are large enough */
        if (ctx->pid_count + cmd_count > ctx->pid_capacity) {
            ctx->pid_capacity = ctx->pid_count + cmd_count;
            ctx->pids = realloc(ctx->pids, ctx->pid_capacity * sizeof(pid_t));
            if (!ctx->pids) {
                perror("realloc pids");
                ctx->has_error = 1;
                break;
            }
        }
        
        if (ctx->thread_count + cmd_count > ctx->thread_capacity) {
            ctx->thread_capacity = ctx->thread_count + cmd_count;
            ctx->threads = realloc(ctx->threads, 
                                  ctx->thread_capacity * sizeof(pthread_t));
            if (!ctx->threads) {
                perror("realloc threads");
                ctx->has_error = 1;
                break;
            }
        }

        /* Execute each command in pipeline */
        ListSimpleCommand sc_list = p->u.pipeLine_.listsimplecommand_;
        int position = 0;
        int last_was_thread = 0;

        while (sc_list) {
            ctx->pipeline_position = position;

            /* Create pipe for all except last command */
            if (position < cmd_count - 1) {
                if (pipe(ctx->curr_pipe) < 0) {
                    perror("pipe");
                    ctx->has_error = 1;
                    break;
                }
            } else {
                ctx->curr_pipe[0] = -1;
                ctx->curr_pipe[1] = -1;
            }
            
            /* Build argv for this command */
            exec_context_reset_command(ctx);
            
            /* Visit word nodes to build argv */
            visitWord(sc_list->simplecommand_->u.cmd_.word_, ctx);
            visitListWord(sc_list->simplecommand_->u.cmd_.listword_, ctx);
            
            /* NULL-terminate argv */
            if (ctx->argc < ctx->argv_capacity) {
                ctx->argv[ctx->argc] = NULL;
            }
            
            if (ctx->argc == 0) {
                fprintf(stderr, "Empty command in pipeline\n");
                ctx->has_error = 1;
                break;
            }
            
            /* Detect if built-in or external */
            const cmd_spec_t *spec = find_command(ctx->argv[0]);
            int is_builtin = (spec != NULL);
            
            if (is_builtin) {
                /* === USE THREAD for built-in === */
                
                /* Determine stdin fd for thread */
                int thread_stdin_fd = -1;
                if (ctx->prev_pipe[0] != -1) {
                    thread_stdin_fd = ctx->prev_pipe[0];
                }
                
                /* Determine stdout fd for thread */
                int thread_stdout_fd = -1;
                if (ctx->curr_pipe[1] != -1) {
                    thread_stdout_fd = ctx->curr_pipe[1];
                }
                
                /* Create thread arguments */
                thread_args_t *targs = thread_args_create(
                    ctx->argc,
                    ctx->argv,
                    thread_stdin_fd,
                    thread_stdout_fd,
                    spec
                );
                
                if (!targs) {
                    fprintf(stderr, "Failed to create thread args\n");
                    ctx->has_error = 1;
                    break;
                }
                
                /* Create thread */
                pthread_t tid;
                if (pthread_create(&tid, NULL, thread_execute_builtin, targs) != 0) {
                    perror("pthread_create");
                    thread_args_free(targs);
                    ctx->has_error = 1;
                    break;
                }
                
                /* Save thread ID */
                ctx->threads[ctx->thread_count++] = tid;
                last_was_thread = 1;
                
                /* Close previous pipe in parent thread
                 * (thread has its own copy of the fd) */
                if (ctx->prev_pipe[0] != -1) {
                    close(ctx->prev_pipe[0]);
                    close(ctx->prev_pipe[1]);
                }
                
            } else {
                /* === USE FORK/EXEC for external command === */
                
                pid_t pid = fork();
                
                if (pid < 0) {
                    perror("fork");
                    ctx->has_error = 1;
                    break;
                }
                
                if (pid == 0) {
                    /* === CHILD PROCESS === */
                    
                    /* Connect stdin to previous pipe */
                    if (ctx->prev_pipe[0] != -1) {
                        dup2(ctx->prev_pipe[0], STDIN_FILENO);
                        close(ctx->prev_pipe[0]);
                        close(ctx->prev_pipe[1]);
                    }
                    
                    /* Connect stdout to current pipe */
                    if (ctx->curr_pipe[1] != -1) {
                        dup2(ctx->curr_pipe[1], STDOUT_FILENO);
                        close(ctx->curr_pipe[0]);
                        close(ctx->curr_pipe[1]);
                    }
                    
                    /* Execute external command */
                    execvp(ctx->argv[0], ctx->argv);
                    perror(ctx->argv[0]);
                    _exit(127);
                }
                
                /* === PARENT PROCESS === */
                
                /* Save PID */
                ctx->pids[ctx->pid_count++] = pid;
                last_was_thread = 0;
                
                /* Close previous pipe in parent */
                if (ctx->prev_pipe[0] != -1) {
                    close(ctx->prev_pipe[0]);
                    close(ctx->prev_pipe[1]);
                }
            }
            
            /* Current pipe becomes previous for next iteration */
            ctx->prev_pipe[0] = ctx->curr_pipe[0];
            ctx->prev_pipe[1] = ctx->curr_pipe[1];
            
            /* Move to next command */
            sc_list = sc_list->listsimplecommand_;
            position++;
        }

        /* Close last pipe in parent */
        if (ctx->prev_pipe[0] != -1) {
            close(ctx->prev_pipe[0]);
            close(ctx->prev_pipe[1]);
        }

        /* Wait for all threads */
        for (int i = 0; i < ctx->thread_count; i++) {
            void *thread_status;
            if (pthread_join(ctx->threads[i], &thread_status) != 0) {
                perror("pthread_join");
            } else {
                /* Last thread's status might be pipeline status */
                if (i == ctx->thread_count - 1 && last_was_thread) {
                    ctx->exit_status = (int)(long)thread_status;
                }
            }
        }
        
        /* Wait for all child processes */
        for (int i = 0; i < ctx->pid_count; i++) {
            int status;
            if (waitpid(ctx->pids[i], &status, 0) >= 0) {
                /* Last process status might be pipeline status */
                if (i == ctx->pid_count - 1 && !last_was_thread) {
                    if (WIFEXITED(status))
                        ctx->exit_status = WEXITSTATUS(status);
                    else if (WIFSIGNALED(status))
                        ctx->exit_status = 128 + WTERMSIG(status);
                }
            }
        }

        /* Reset pipeline state */
        ctx->pid_count = 0;
        ctx->thread_count = 0;
        ctx->in_pipeline = 0;

        break;
    }

    default:
        fprintf(stderr, "Error: bad kind field when visiting Pipeline!\n");
        exit(1);
    }
}
Compile & Test:
bashmake clean && make

./picobox
$ ls | wc -l
<should print line count>

$ cat /etc/hosts | head -n 5
<should print first 5 lines>

$ pwd | cat
<should print current directory>

$ exit
Test Checklist:

 Simple pipeline works: ls | wc
 All built-in: pwd | cat | head
 Mixed: /bin/ls | wc | cat
 Long pipeline: ls | head | cat | wc
 No deadlocks
 No memory leaks (valgrind)
 Output is correct (not corrupted)


PHASE 3: COMPREHENSIVE TESTING
Step 3.1: Create Test Scripts
Create: tests/test_all.sh
bash#!/bin/bash

echo "======================================"
echo "PICOBOX SHELL TEST SUITE"
echo "======================================"

TESTS_PASSED=0
TESTS_FAILED=0

# Function to run a test
run_test() {
    local test_name="$1"
    local test_cmd="$2"
    local expected="$3"
    
    echo -n "Testing: $test_name... "
    
    # Run test
    result=$(echo "$test_cmd" | ./picobox 2>&1)
    
    if [[ "$result" == *"$expected"* ]]; then
        echo "PASS"
        ((TESTS_PASSED++))
    else
        echo "FAIL"
        echo "  Expected: $expected"
        echo "  Got: $result"
        ((TESTS_FAILED++))
    fi
}

echo ""
echo "=== VARIABLE TESTS ==="

run_test "Simple assignment" "myvar=hello; echo \$myvar" "hello"
run_test "Reassignment" "var=1; var=2; echo \$var" "2"
run_test "Environment var" "echo \$HOME" "$HOME"
run_test "Special var $$" "echo \$\$" "[0-9]"
run_test "Undefined var" "echo \$NOTEXIST" ""
run_test "Export" "export TEST=value; echo \$TEST" "value"

echo ""
echo "=== THREAD PIPELINE TESTS ==="

run_test "Built-in pipeline" "ls | wc -l" "[0-9]"
run_test "Mixed pipeline" "/bin/echo test | cat" "test"
run_test "Long pipeline" "echo hello | cat | cat | cat" "hello"

echo ""
echo "======================================"
echo "RESULTS: $TESTS_PASSED passed, $TESTS_FAILED failed"
echo "======================================"

if [ $TESTS_FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi
Make executable:
bashchmod +x tests/test_all.sh
Run tests:
bash./tests/test_all.sh

Step 3.2: Memory Leak Testing
Create: tests/test_leaks.sh
bash#!/bin/bash

echo "Running memory leak tests with valgrind..."

# Test 1: Variable operations
echo "Test 1: Variable operations"
valgrind --leak-check=full --error-exitcode=1 \
    ./picobox <<EOF
myvar=test
echo \$myvar
export myvar
exit
EOF

# Test 2: Pipelines
echo "Test 2: Pipelines"
valgrind --leak-check=full --error-exitcode=1 \
    ./picobox <<EOF
ls | wc
cat /etc/hosts | head
exit
EOF

echo "Memory tests complete!"
Make executable and run:
bashchmod +x tests/test_leaks.sh
./tests/test_leaks.sh

Step 3.3: Stress Testing
Create: tests/test_stress.sh
bash#!/bin/bash

echo "Running stress tests..."

# Test 1: Many variables
echo "Test 1: Creating 1000 variables"
./picobox <<EOF
$(for i in {1..1000}; do echo "var$i=value$i"; done)
echo \$var1000
exit
EOF

# Test 2: Many pipelines
echo "Test 2: Running 100 pipelines"
./picobox <<EOF
$(for i in {1..100}; do echo "echo test | cat | wc"; done)
exit
EOF

# Test 3: Long pipeline
echo "Test 3: Long pipeline (10 stages)"
./picobox <<EOF
echo test | cat | cat | cat | cat | cat | cat | cat | cat | cat
exit
EOF

echo "Stress tests complete!"
Make executable and run:
bashchmod +x tests/test_stress.sh
./tests/test_stress.sh

FINAL CHECKLIST
Code Quality:

 All files compile without warnings
 All functions have header comments
 All mallocs have corresponding frees
 All file descriptors are closed
 No global mutable state (except command registry)
 Error messages are clear and helpful

Functionality:

 Shell variables work (set and get)
 Environment variables work (export)
 Variable expansion works ($VAR)
 Special variables work ($$, $?, $0)
 Thread-based pipelines work
 Mixed pipelines work (threads + processes)
 Long pipelines work (5+ stages)

Testing:

 All test scripts pass
 Valgrind shows no leaks
 Valgrind shows no errors
 Stress tests pass
 No deadlocks in pipelines

Documentation:

 README.md updated
 CHANGELOG.md updated
 Code comments are clear


CONGRATULATIONS! If all checkboxes are checked, implementation is complete.Claude is AI and can make mistakes. Please double-check responses. Sonnet 4.5Claude is AI and can make mistakes. Please double-check responses.