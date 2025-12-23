plan.md
Implementation Plan: Shell Variables & Thread-Based Pipeline Execution
Overview
This plan implements two major features for the PicoBox BNFC shell:

Shell Variables: Both environment variables ($PATH, $HOME) and shell-local variables ($myvar)
Thread-Based Execution: Replace fork/exec with pthreads for built-in commands in pipelines


PHASE 1: Shell Variables Infrastructure
1.1 Create Variable Storage System
Files to Create:

include/var_table.h - Variable table interface
src/var_table.c - Hash table implementation for shell variables

Objective: Implement efficient variable storage with separate mechanisms for environment vs shell-local variables.
Design Decision:

Environment variables: Use getenv()/setenv() directly (system-managed)
Shell-local variables: Hash table (our custom storage)
Why separate?: Environment variables persist across processes (needed for child processes), shell variables are shell-session only

Hash Table Specifications:
ctypedef struct var_entry {
    char *name;           // Variable name (without $)
    char *value;          // Variable value (always string)
    struct var_entry *next;  // Collision chain
} var_entry_t;

typedef struct var_table {
    var_entry_t **buckets;  // Array of hash buckets
    size_t size;            // Number of buckets (default: 128)
    size_t count;           // Number of variables stored
} var_table_t;
Functions Required:
cvar_table_t* var_table_create(size_t size);
void var_table_destroy(var_table_t *table);
int var_table_set(var_table_t *table, const char *name, const char *value);
const char* var_table_get(var_table_t *table, const char *name);
int var_table_unset(var_table_t *table, const char *name);
Testing Checkpoints:

✓ Create empty hash table
✓ Insert 100 variables, verify retrieval
✓ Test collision handling
✓ Test update existing variable
✓ Test delete variable
✓ Test memory cleanup (valgrind)


1.2 Integrate Variable Table into ExecContext
File to Modify:

include/Skeleton.h
src/bnfc/Skeleton.c

Changes to Skeleton.h:
c#include "var_table.h"  // Add this include

typedef struct exec_context {
    /* Existing fields... */
    
    /* NEW: Variable storage */
    var_table_t *shell_vars;   // Shell-local variables ($myvar)
    // Note: Environment variables use getenv/setenv directly
    
} ExecContext;
Changes to exec_context_new():
cExecContext *exec_context_new(void)
{
    ExecContext *ctx = calloc(1, sizeof(ExecContext));
    if (!ctx) {
        perror("calloc");
        return NULL;
    }
    
    /* ... existing initialization ... */
    
    /* Initialize variable table */
    ctx->shell_vars = var_table_create(128);
    if (!ctx->shell_vars) {
        /* cleanup and return NULL */
    }
    
    return ctx;
}
Changes to exec_context_free():
cvoid exec_context_free(ExecContext *ctx)
{
    if (!ctx) return;
    
    /* ... existing cleanup ... */
    
    /* Free variable table */
    var_table_destroy(ctx->shell_vars);
    
    free(ctx);
}
Testing Checkpoints:

✓ Context creates with variable table
✓ Context frees without memory leaks
✓ Multiple contexts can exist independently


1.3 Implement Variable Expansion in visitWord()
File to Modify:

src/bnfc/Skeleton.c - visitWord() function

Current Implementation:
cvoid visitWord(Word p, ExecContext *ctx)
{
    /* Add word to argv */
    ctx->argv[ctx->argc] = strdup(p);
    ctx->argc++;
}
New Implementation with Variable Expansion:
c/*
 * Expand variable references in a word
 * 
 * Examples:
 *   "$HOME"        -> "/home/user"
 *   "$myvar"       -> "hello"
 *   "prefix$var"   -> "prefixvalue"
 *   "$$"           -> process ID
 *   "$?"           -> last exit status
 * 
 * Algorithm:
 *   1. Scan for $ characters
 *   2. Extract variable name (alphanumeric + underscore)
 *   3. Look up in shell_vars, then environment
 *   4. Replace $var with value
 *   5. Handle special variables ($$, $?, $0, etc.)
 */
static char* expand_variables(const char *word, ExecContext *ctx)
{
    char *result = malloc(4096);  // Max expanded size
    char *dst = result;
    const char *src = word;
    
    while (*src) {
        if (*src == '$') {
            src++;  // Skip $
            
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
            }
            
            /* Extract variable name */
            char varname[256];
            int i = 0;
            while (*src && (isalnum(*src) || *src == '_')) {
                varname[i++] = *src++;
            }
            varname[i] = '\0';
            
            if (i == 0) {
                /* Just a $ with nothing after - keep it literal */
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
                strcpy(dst, value);
                dst += strlen(value);
            }
            /* If not found, expand to empty string (standard behavior) */
            
        } else {
            /* Regular character - copy as-is */
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    return result;
}

void visitWord(Word p, ExecContext *ctx)
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
Testing Checkpoints:

✓ Echo literal text: echo hello → "hello"
✓ Expand environment variable: echo $HOME → "/home/user"
✓ Expand shell variable: echo $myvar → (value of myvar)
✓ Expand special variables: echo $$ → process ID
✓ Expand in middle: echo prefix$HOME/suffix → "prefix/home/user/suffix"
✓ Undefined variable: echo $NOTEXIST → "" (empty)
✓ Literal dollar: echo $$ vs echo \$ (if escaping added later)


1.4 Add Variable Assignment Command (Built-in)
Objective: Support shell-local variable assignment: myvar=value
Implementation Strategy: Add new visitor case or detect assignment pattern in visitSimpleCommand()
Option A: Detect in visitSimpleCommand() (RECOMMENDED)
Modify visitSimpleCommand() to detect VAR=VALUE pattern:
cvoid visitSimpleCommand(SimpleCommand p, ExecContext *ctx)
{
    switch(p->kind)
    {
    case is_Cmd:
        /* Reset command state for new command */
        exec_context_reset_command(ctx);

        /* Build argv by visiting word nodes */
        visitWord(p->u.cmd_.word_, ctx);
        visitListWord(p->u.cmd_.listword_, ctx);
        
        /* NULL-terminate argv */
        if (ctx->argc < ctx->argv_capacity) {
            ctx->argv[ctx->argc] = NULL;
        }
        
        /* Check if no command */
        if (ctx->argc == 0 || !ctx->argv[0]) {
            break;
        }
        
        /* NEW: Check for variable assignment (VAR=VALUE) */
        if (strchr(ctx->argv[0], '=') && !strchr(ctx->argv[0], '/')) {
            /* This looks like an assignment */
            char *equals = strchr(ctx->argv[0], '=');
            *equals = '\0';  // Split at =
            char *name = ctx->argv[0];
            char *value = equals + 1;
            
            /* Validate variable name (must start with letter/underscore) */
            if (!isalpha(name[0]) && name[0] != '_') {
                fprintf(stderr, "Invalid variable name: %s\n", name);
                ctx->exit_status = EXIT_ERROR;
                break;
            }
            
            /* Set the variable */
            var_table_set(ctx->shell_vars, name, value);
            ctx->exit_status = EXIT_OK;
            break;  // Don't execute as command
        }
        
        /* Visit redirections and continue with normal execution... */
Testing Checkpoints:

✓ Simple assignment: myvar=hello
✓ Assignment with spaces in value: myvar="hello world"
✓ Assignment then use: myvar=test; echo $myvar → "test"
✓ Reassignment: myvar=1; myvar=2; echo $myvar → "2"
✓ Invalid names rejected: 123var=test → error
✓ Assignment doesn't interfere with commands containing =: /bin/ls=test → runs /bin/ls=test


1.5 Add Export Built-in (Environment Variables)
File to Modify:

src/bnfc/Skeleton.c

Add to visitSimpleCommand():
c/* Check for built-ins that MUST run in parent */
if (strcmp(ctx->argv[0], "export") == 0) {
    ctx->exit_status = builtin_export(ctx);
    break;
}
New helper function:
c/*
 * export - Set environment variable
 * 
 * Usage:
 *   export VAR=VALUE       - Set and export
 *   export VAR             - Export existing shell variable
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
            
            if (setenv(name, value, 1) != 0) {
                perror("export");
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
                perror("export");
                return EXIT_ERROR;
            }
        }
    }
    
    return EXIT_OK;
}
```

**Testing Checkpoints:**
- ✓ Export with value: `export PATH=/usr/bin`
- ✓ Export shell var: `myvar=test; export myvar`
- ✓ Child sees exported: `export TEST=hello; /bin/sh -c 'echo $TEST'` → "hello"
- ✓ Child doesn't see shell-local: `myvar=test; /bin/sh -c 'echo $myvar'` → ""

---

## PHASE 2: Thread-Based Pipeline Execution

### 2.1 Understand Current vs Target Architecture

**Current Architecture (Fork/Exec for everything):**
```
ls | grep test | wc
 ↓     ↓          ↓
fork  fork       fork
exec  exec       exec
```

**Target Architecture (Threads for built-ins):**
```
ls | grep test | wc
 ↓     ↓          ↓
fork  thread     fork  (if ls, wc are external; grep is built-in)

OR if all built-in:

myls | mygrep | mywc
  ↓      ↓       ↓
thread thread  thread
Key Insight from Lecture:

Threads share file descriptors ✓ (pipes still work!)
Threads share memory ✓ (same ExecContext)
Each thread needs own stack ✓ (automatic with pthread_create)
Must use pthread_join() instead of waitpid()


2.2 Add Thread Support to ExecContext
File to Modify:

include/Skeleton.h
src/bnfc/Skeleton.c

Add to Skeleton.h:
c#include <pthread.h>

typedef struct exec_context {
    /* Existing fields... */
    
    /* Thread tracking (parallel to PID tracking) */
    pthread_t *threads;      // Array of thread IDs
    int thread_count;        // Number of threads created
    int thread_capacity;     // Allocated capacity
    
    /* Mixed pipeline support */
    int using_threads;       // Flag: is this pipeline using threads?
    
} ExecContext;
Add to exec_context_new():
c/* Initialize thread tracking array */
ctx->thread_capacity = 8;
ctx->threads = malloc(ctx->thread_capacity * sizeof(pthread_t));
if (!ctx->threads) {
    free(ctx->pids);
    free(ctx->argv);
    free(ctx);
    return NULL;
}
ctx->thread_count = 0;
ctx->using_threads = 0;
Add to exec_context_free():
c/* Free thread array */
free(ctx->threads);

2.3 Create Thread Execution Function
File to Create:

src/thread_exec.c
include/thread_exec.h

thread_exec.h:
c#ifndef THREAD_EXEC_H
#define THREAD_EXEC_H

#include "Skeleton.h"

/*
 * Thread argument structure
 * Passed to pthread_create - contains everything thread needs
 */
typedef struct thread_args {
    ExecContext *ctx;        // Execution context (SHARED across threads!)
    int argc;                // Copy of argc
    char **argv;             // Copy of argv (THREAD-PRIVATE)
    int stdin_fd;            // File descriptor for stdin
    int stdout_fd;           // File descriptor for stdout
    const cmd_spec_t *spec;  // Command spec to execute
} thread_args_t;

/*
 * Thread entry point for built-in command execution
 * This is what pthread_create() calls
 */
void* thread_execute_builtin(void *arg);

/*
 * Helper: Create thread args structure
 */
thread_args_t* thread_args_create(ExecContext *ctx, const cmd_spec_t *spec);

/*
 * Helper: Free thread args structure
 */
void thread_args_free(thread_args_t *args);

#endif /* THREAD_EXEC_H */
thread_exec.c:
c#include "thread_exec.h"
#include "picobox.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/*
 * Thread entry point for built-in command execution
 * 
 * This function runs in a separate thread. It:
 * 1. Redirects stdin/stdout to pipe file descriptors
 * 2. Calls the built-in command function directly (no exec!)
 * 3. Returns exit status via pthread_exit()
 * 
 * CRITICAL: This runs in the SAME ADDRESS SPACE as parent
 * - File descriptors are shared (pipes work!)
 * - Must NOT call exit() - use pthread_exit() instead
 * - Must free thread-private memory before exiting
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
    
    /* Flush output streams (important!) */
    fflush(stdout);
    fflush(stderr);
    
cleanup:
    /* Free thread-private memory */
    thread_args_free(targs);
    
    /* Return exit status (pthread_join can retrieve this) */
    pthread_exit((void *)(long)status);
    
    return NULL;  /* Never reached */
}

/*
 * Create thread args structure
 * Makes COPIES of argc/argv so thread has private data
 */
thread_args_t* thread_args_create(ExecContext *ctx, const cmd_spec_t *spec)
{
    thread_args_t *args = malloc(sizeof(thread_args_t));
    if (!args) {
        perror("malloc");
        return NULL;
    }
    
    args->ctx = ctx;  // Shared context
    args->spec = spec;
    args->argc = ctx->argc;
    
    /* Copy argv (thread needs private copy) */
    args->argv = malloc((ctx->argc + 1) * sizeof(char *));
    if (!args->argv) {
        free(args);
        return NULL;
    }
    
    for (int i = 0; i < ctx->argc; i++) {
        args->argv[i] = strdup(ctx->argv[i]);
        if (!args->argv[i]) {
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(args->argv[j]);
            }
            free(args->argv);
            free(args);
            return NULL;
        }
    }
    args->argv[ctx->argc] = NULL;
    
    /* Copy file descriptors */
    args->stdin_fd = ctx->stdin_fd;
    args->stdout_fd = ctx->stdout_fd;
    
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

2.4 Modify visitPipeline() for Thread Support
File to Modify:

src/bnfc/Skeleton.c - visitPipeline() function

Strategy:

Detect if command is built-in or external
For built-ins: Use pthread_create() instead of fork()
For external: Still use fork()/exec()
Mixed pipelines: Support both in same pipeline
Use pthread_join() for threads, waitpid() for processes

Key Changes:
cvoid visitPipeline(Pipeline p, ExecContext *ctx)
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
            ctx->threads = realloc(ctx->threads, ctx->thread_capacity * sizeof(pthread_t));
            if (!ctx->threads) {
                perror("realloc threads");
                ctx->has_error = 1;
                break;
            }
        }

        /* Execute each command in pipeline */
        ListSimpleCommand sc_list = p->u.pipeLine_.listsimplecommand_;
        int position = 0;

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
            visitWord(sc_list->simplecommand_->u.cmd_.word_, ctx);
            visitListWord(sc_list->simplecommand_->u.cmd_.listword_, ctx);
            
            if (ctx->argc < ctx->argv_capacity) {
                ctx->argv[ctx->argc] = NULL;
            }
            
            if (ctx->argc == 0) {
                fprintf(stderr, "Empty command in pipeline\n");
                ctx->has_error = 1;
                break;
            }
            
            /* Check if this is a built-in command */
            const cmd_spec_t *spec = find_command(ctx->argv[0]);
            int is_builtin = (spec != NULL);
            
            if (is_builtin) {
                /* === USE THREAD for built-in === */
                
                /* Set up file descriptors for thread */
                if (ctx->prev_pipe[0] != -1) {
                    ctx->stdin_fd = ctx->prev_pipe[0];
                } else {
                    ctx->stdin_fd = -1;
                }
                
                if (ctx->curr_pipe[1] != -1) {
                    ctx->stdout_fd = ctx->curr_pipe[1];
                } else {
                    ctx->stdout_fd = -1;
                }
                
                /* Create thread arguments */
                thread_args_t *targs = thread_args_create(ctx, spec);
                if (!targs) {
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
                ctx->using_threads = 1;
                
                /* Close pipe ends in parent thread */
                if (ctx->prev_pipe[0] != -1) {
                    close(ctx->prev_pipe[0]);
                    close(ctx->prev_pipe[1]);
                }
                
                /* IMPORTANT: Close current pipe write end in parent
                 * Thread has its own copy of the fd */
                if (ctx->curr_pipe[1] != -1) {
                    close(ctx->curr_pipe[1]);
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
                /* Last thread's status is pipeline status */
                if (i == ctx->thread_count - 1) {
                    ctx->exit_status = (int)(long)thread_status;
                }
            }
        }
        
        /* Wait for all child processes */
        for (int i = 0; i < ctx->pid_count; i++) {
            int status;
            if (waitpid(ctx->pids[i], &status, 0) >= 0) {
                /* If this was the last command, use its status */
                if (i == ctx->pid_count - 1 && ctx->thread_count == 0) {
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
        ctx->using_threads = 0;

        break;
    }

    default:
        fprintf(stderr, "Error: bad kind field when visiting Pipeline!\n");
        exit(1);
    }
}
Testing Checkpoints:

✓ All built-in pipeline: ls | wc | head (if all are built-in)
✓ Mixed pipeline: /bin/ls | wc | /bin/cat (external | builtin | external)
✓ Thread pipe synchronization works (no deadlocks)
✓ Thread exit status propagates correctly
✓ No memory leaks (valgrind)
✓ Concurrent stdout writes don't corrupt (use printf for atomic writes)


2.5 Handle Thread Safety Issues
Potential Issues:

printf() is thread-safe (POSIX guarantees atomic writes for complete format strings)
File descriptors are shared (threads can close each other's FDs - BE CAREFUL)
ExecContext is shared (but only read during thread execution - OK)

Safety Measures:

Thread gets PRIVATE argv copy ✓ (already in thread_args_create)
Thread gets COPIES of file descriptors ✓ (stdin_fd, stdout_fd copied)
Parent closes pipe ends AFTER thread creation ✓ (see visitPipeline)
Mutex for error reporting (if needed):

c/* Add to ExecContext if we see race conditions */
pthread_mutex_t error_mutex;  // Protects has_error, exit_status
Testing Checkpoints:

✓ Run same pipeline 1000 times: for i in {1..1000}; do ls | wc; done
✓ No corrupted output
✓ No fd leaks: lsof -p $$ before and after
✓ No thread leaks: Check thread count doesn't grow


PHASE 3: Integration & Testing
3.1 Comprehensive Test Suite
Test Script: tests/test_variables.sh
bash#!/bin/bash

echo "=== Testing Shell Variables ==="

# Test 1: Simple assignment and expansion
echo "Test 1: myvar=hello; echo \$myvar"
# Expected: hello

# Test 2: Environment variable
echo "Test 2: export MYTEST=world; echo \$MYTEST"
# Expected: world

# Test 3: Variable in pipeline
echo "Test 3: myvar=test; echo \$myvar | wc -c"
# Expected: 5 (test + newline)

# Test 4: Special variables
echo "Test 4: echo \$\$ \$?"
# Expected: <pid> 0

# Test 5: Undefined variable
echo "Test 5: echo \$UNDEFINED"
# Expected: (empty line)

# ... more tests ...
Test Script: tests/test_threads.sh
bash#!/bin/bash

echo "=== Testing Thread-Based Pipelines ==="

# Test 1: All built-in pipeline
echo "Test 1: ls | wc -l"
# Expected: line count

# Test 2: Mixed pipeline
echo "Test 2: /bin/cat file.txt | wc | head -n 1"
# Expected: wc output

# Test 3: Long pipeline
echo "Test 3: ls | head | wc | cat"
# Expected: wc output

# Test 4: Stress test
echo "Test 4: Running 100 pipelines..."
for i in {1..100}; do
    ls | wc > /dev/null
done
echo "Done"

# ... more tests ...
3.2 Makefile Integration
Add to Makefile:
makefile# Test targets
test: test-vars test-threads

test-vars:
	@echo "Running variable tests..."
	@./tests/test_variables.sh

test-threads:
	@echo "Running thread tests..."
	@./tests/test_threads.sh

test-mem:
	@echo "Running memory leak tests..."
	@valgrind --leak-check=full ./picobox < tests/test_input.txt

PHASE 4: Documentation & Cleanup
4.1 Update Documentation
Files to Update:

README.md - Add variables and threading features
CHANGELOG.md - Document changes
Add inline comments to all new functions

4.2 Code Review Checklist

 All functions have header comments
 All memory allocations have corresponding frees
 All file descriptors are closed
 No global variables (except command registry)
 Thread-safe code (no shared mutable state)
 Error handling for all syscalls
 valgrind clean (no leaks, no errors)


Implementation Order Summary

PHASE 1: Variables (Estimated: 2-3 days)

1.1 Hash table implementation
1.2 Integrate into ExecContext
1.3 Variable expansion in visitWord()
1.4 Assignment detection
1.5 Export built-in


PHASE 2: Threads (Estimated: 3-4 days)

2.1 Study architecture
2.2 Add thread tracking to ExecContext
2.3 Create thread_exec.c
2.4 Modify visitPipeline()
2.5 Thread safety audit


PHASE 3: Testing (Estimated: 1-2 days)

3.1 Write comprehensive tests
3.2 Fix bugs found during testing


PHASE 4: Polish (Estimated: 1 day)

4.1 Documentation
4.2 Code review and cleanup



Total Estimated Time: 7-10 days

Safety & Quality Requirements
Must Test After Every Change:

Compile without warnings: make clean && make
Run basic tests: echo hello | cat
Check for leaks: valgrind ./picobox
Verify no fd leaks: lsof -p $$

Must Not Continue If:

Code doesn't compile
Valgrind shows errors
Basic commands are broken
Threads deadlock or hang

Code Quality Standards:

Every function has a comment block
Every malloc has a corresponding free
Every open has a corresponding close
Error messages are clear and helpful
No magic numbers (use #define)


END OF PLAN.MD