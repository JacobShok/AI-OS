# BNFC Shell Implementation Plan
## Incremental Development with Continuous Testing

**Version:** 1.0  
**Target:** Transform PicoBox simple shell into BNFC-powered shell with pipes and redirections  
**Philosophy:** Test every single step. No blind coding. Each phase must pass tests before moving to the next.

---

## 📋 Table of Contents

1. [Current State Analysis](#phase-0-current-state-analysis)
2. [Prerequisites & Setup](#phase-1-prerequisites--setup)
3. [BNFC Calculator Learning](#phase-2-bnfc-calculator-learning)
4. [Simple Grammar Integration](#phase-3-simple-grammar-integration)
5. [Fork/Exec Implementation](#phase-4-forkexec-implementation)
6. [Pipes Implementation](#phase-5-pipes-implementation)
7. [Redirections Implementation](#phase-6-redirections-implementation)
8. [Background Jobs](#phase-7-background-jobs)
9. [Integration & Polish](#phase-8-integration--polish)
10. [Testing Strategy](#testing-strategy)

---

## PHASE 0: Current State Analysis

### What We Have ✅
```
picobox/
├── main.c              # Entry point with command dispatcher
├── shell.c             # Simple REPL shell (IN-PROCESS execution)
├── utils.c/h           # Helper functions
├── picobox.h           # Main header
├── cmd_*.c             # 25+ individual command implementations
└── Makefile            # Build system
```

### Current Shell Capabilities
- ✅ Command dispatcher (finds functions by name)
- ✅ Built-in commands: `cd`, `exit`, `help`
- ✅ Basic line parsing (handles quotes, whitespace)
- ✅ IN-PROCESS execution (calls functions directly)
- ❌ NO fork/exec (all commands run in same process)
- ❌ NO pipes
- ❌ NO redirections
- ❌ NO background jobs
- ❌ NO grammar-based parsing

### Critical Insights
1. **Current execution model is WRONG for a real shell**
   - Commands run as function calls, not separate processes
   - `cat` reads files directly in shell process
   - No process isolation
   
2. **Need to preserve existing command implementations**
   - These will become the fallback for symlink mode
   - Shell mode will use fork/exec instead

3. **BNFC will replace `parse_line()` function**
   - Current: Manual tokenization with quotes
   - Future: BNFC lexer → parser → AST

---

## PHASE 1: Prerequisites & Setup

**Goal:** Install BNFC and verify toolchain works  
**Time Estimate:** 30-60 minutes  
**Success Criteria:** Can compile and run calculator example

### 1.1 Install BNFC

```bash
# Option 1: Using package manager (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install bnfc

# Option 2: From releases (if package not available)
wget https://github.com/BNFC/bnfc/releases/download/v2.9.6/bnfc-2.9.6-linux.binary
chmod +x bnfc-2.9.6-linux.binary
sudo mv bnfc-2.9.6-linux.binary /usr/local/bin/bnfc

# Verify installation
bnfc --version
```

### 1.2 Install Required Tools

```bash
# Install Flex, Bison, and build tools
sudo apt-get install flex bison build-essential

# Verify installations
flex --version
bison --version
gcc --version
```

### 1.3 Create Project Structure

```bash
# In your picobox directory
mkdir -p bnfc_shell/
mkdir -p tests/cases/
mkdir -p tests/expected/
mkdir -p docs/
```

### 1.4 Test Files

Create initial test directory structure:

```bash
# Create test organization
tests/
├── cases/               # Input test cases
│   ├── 00_empty.txt
│   ├── 01_simple.txt
│   ├── 02_pipe.txt
│   └── ...
├── expected/            # Expected outputs
│   ├── 00_empty.txt
│   ├── 01_simple.txt
│   └── ...
└── run_tests.sh        # Test runner script
```

### ✅ Verification Tests

**Test 1.1: BNFC Installation**
```bash
# Should output version info
bnfc --version
# Expected: BNFC version 2.9.x
```

**Test 1.2: Flex/Bison Installation**
```bash
flex --version && bison --version
# Expected: Both tools report versions
```

**Test 1.3: Directory Structure**
```bash
ls -la bnfc_shell/ tests/cases/ tests/expected/
# Expected: All directories exist
```

**📊 Phase 1 Checklist:**
- [ ] BNFC installed and version confirmed
- [ ] Flex and Bison installed
- [ ] Directory structure created
- [ ] Can access teacher's provided files
- [ ] All verification tests pass

---

## PHASE 2: BNFC Calculator Learning

**Goal:** Fully understand BNFC workflow with calculator example  
**Time Estimate:** 2-3 hours  
**Success Criteria:** Can modify calculator and see results

### 2.1 Setup Calculator Example

Create `bnfc_shell/Calc.cf`:

```bnfc
-- BNFC tutorial-inspired calculator (C backend only)
-- Precedence via Exp/Term/Factor split
comment "/*" "*/" ;
comment "//" "\n" ;

entrypoints Exp ;

EAdd.  Exp   ::= Exp "+" Term ;
ESub.  Exp   ::= Exp "-" Term ;
ToExp. Exp   ::= Term ;

EMul.  Term  ::= Term "*" Factor ;
EDiv.  Term  ::= Term "/" Factor ;
ToTerm. Term ::= Factor ;

EInt.  Factor ::= Integer ;
EVar.  Factor ::= Ident ;
EPar.  Factor ::= "(" Exp ")" ;
```

### 2.2 Generate and Build

```bash
cd bnfc_shell/

# Generate all files
bnfc --c Calc.cf

# Examine generated files
ls -la
# You should see:
# - Absyn.c, Absyn.h      (AST definitions)
# - Lexer.c, Lexer.h      (Tokenizer)
# - Parser.c, Parser.h    (Parser)
# - Printer.c, Printer.h  (Pretty printer)
# - Skeleton.c, Skeleton.h (Visitor template)
# - Test.c                 (Test driver)
# - Makefile              (Build rules)

# Build
make

# Should create TestCalc executable
```

### 2.3 Test Calculator (Understanding Phase)

```bash
# Test 1: Simple expression
echo "1 + 2" | ./TestCalc
# Expected output:
# Parse Successful!
# [Abstract Syntax]
# (EAdd (ToExp (ToTerm (EInt 1))) (ToTerm (EInt 2)))
# [Linearized tree]
# 1 + 2

# Test 2: Precedence
echo "1 + 2 * 3" | ./TestCalc
# Expected: Shows that * binds before +

# Test 3: Parentheses
echo "(1 + 2) * 3" | ./TestCalc
# Expected: Different AST structure than above
```

### 2.4 Study Generated Files

**Study Checklist:**
```bash
# 1. Look at AST definitions
cat Absyn.h | less
# Understand: Exp, Term, Factor structs
# Understand: kind enum (is_EAdd, is_EMul, etc.)

# 2. Look at Parser interface
cat Parser.h
# Find: pExp(FILE *inp) - main parsing function
# Find: psExp(const char *str) - parse from string

# 3. Look at Skeleton template
cat Skeleton.c
# Understand: visitExp(), visitTerm(), visitFactor()
# Understand: Switch on p->kind
# Understand: Accessing children via p->u.eAdd_.exp_, etc.

# 4. Look at Test driver
cat Test.c
# Understand: How pExp() is called
# Understand: How showExp() and printExp() work
```

### 2.5 Implement Calculator Evaluation

**Modify `Skeleton.c` to actually calculate:**

```c
// At top of Skeleton.c, add:
#include <stdio.h>

// Global stack for evaluation
static int eval_stack[1000];
static int eval_sp = 0;

void visitInteger(Integer i)
{
    eval_stack[eval_sp++] = i;
    printf("  [DEBUG] Pushed %d, stack depth = %d\n", i, eval_sp);
}

void visitExp(Exp p)
{
    switch(p->kind)
    {
    case is_EAdd:
        printf("[DEBUG] Evaluating EAdd\n");
        visitExp(p->u.eAdd_.exp_);
        visitTerm(p->u.eAdd_.term_);
        
        int right = eval_stack[--eval_sp];
        int left = eval_stack[--eval_sp];
        int result = left + right;
        eval_stack[eval_sp++] = result;
        printf("  [DEBUG] %d + %d = %d\n", left, right, result);
        break;
        
    case is_ESub:
        printf("[DEBUG] Evaluating ESub\n");
        visitExp(p->u.eSub_.exp_);
        visitTerm(p->u.eSub_.term_);
        
        int right = eval_stack[--eval_sp];
        int left = eval_stack[--eval_sp];
        int result = left - right;
        eval_stack[eval_sp++] = result;
        printf("  [DEBUG] %d - %d = %d\n", left, right, result);
        break;
        
    case is_ToExp:
        visitTerm(p->u.toExp_.term_);
        break;
    }
}

// Similar for visitTerm and visitFactor...
```

**Modify `Test.c` to print result:**

```c
// After the existing code, add:
if (parse_tree)
{
    printf("\nParse Successful!\n");
    
    // Call evaluation
    visitExp(parse_tree);
    
    // Print result
    printf("\n=== RESULT: %d ===\n", eval_stack[0]);
    
    // ... rest of existing code
}
```

### 2.6 Rebuild and Test Evaluation

```bash
make clean
make

echo "1 + 2 * 3" | ./TestCalc
# Should now show:
# - Debug traces of evaluation
# - Final result: 7
```

### ✅ Phase 2 Verification Tests

**Test 2.1: Basic arithmetic**
```bash
echo "5 + 3" | ./TestCalc
# Expected result: 8
```

**Test 2.2: Precedence**
```bash
echo "2 + 3 * 4" | ./TestCalc
# Expected result: 14 (not 20)
```

**Test 2.3: Parentheses**
```bash
echo "(2 + 3) * 4" | ./TestCalc
# Expected result: 20
```

**Test 2.4: Parse error**
```bash
echo "2 +" | ./TestCalc
# Expected: Parse error message
```

**📊 Phase 2 Checklist:**
- [ ] Calculator generates successfully
- [ ] Can build TestCalc
- [ ] Understands Absyn.h structure
- [ ] Understands Skeleton.c visitor pattern
- [ ] Implemented working evaluation
- [ ] All verification tests pass
- [ ] Can explain how token → parse → AST → evaluation works

---

## PHASE 3: Simple Grammar Integration

**Goal:** Create minimal shell grammar and integrate with picobox  
**Time Estimate:** 3-4 hours  
**Success Criteria:** Can parse simple commands via BNFC

### 3.1 Create Initial Shell Grammar

Create `bnfc_shell/Shell.cf`:

```bnfc
-- Minimal shell grammar - Phase 3 (simple commands only)
comment "/*" "*/" ;
comment "//" "\n" ;

-- Token for words (command names, arguments, filenames)
token Word (letter (letter | digit | [._/\-])*) ;

-- Entry point
entry Input ;

-- A shell input is a list of commands
StartInput. Input ::= [Command] ;

-- A command is just a word followed by arguments
SimpleCmd. Command ::= Word [Word] ;

-- Separators
separator Command ";" ;
separator Word " " ;
```

### 3.2 Generate Shell Parser

```bash
cd bnfc_shell/

# Generate shell parser files
bnfc --c Shell.cf

# Build
make

# Test with simple input
echo "cat file.txt" | ./TestShell
# Expected: AST showing SimpleCmd with "cat" and ["file.txt"]
```

### 3.3 Create Shell Integration File

Create `shell_bnfc.c`:

```c
/*
 * shell_bnfc.c - BNFC-powered shell implementation
 */

#include "picobox.h"
#include "../bnfc_shell/Parser.h"
#include "../bnfc_shell/Absyn.h"
#include "../bnfc_shell/Printer.h"
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define PROMPT "$ "

/* External command table */
extern const struct command commands[];

/*
 * Find command function by name
 */
static cmd_func_t find_command(const char *name)
{
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return commands[i].func;
        }
    }
    return NULL;
}

/*
 * Execute a simple command (Phase 3 - no fork/exec yet)
 */
static int execute_simple_command(Command *cmd)
{
    // For now, just print what we would execute
    printf("[DEBUG] Would execute command\n");
    
    // TODO: Will implement fork/exec in Phase 4
    
    return EXIT_OK;
}

/*
 * Execute parsed input
 */
static int execute_input(Input *input)
{
    // For Phase 3, just traverse and print
    printf("[DEBUG] Executing input...\n");
    
    // TODO: Actual execution in later phases
    
    return EXIT_OK;
}

/*
 * Main BNFC shell loop
 */
int shell_bnfc_main(void)
{
    char line[MAX_LINE_LENGTH];
    Input *ast;

    printf("PicoBox BNFC Shell v%s\n", PICOBOX_VERSION);
    printf("Type 'exit' to quit.\n");

    while (1) {
        /* Print prompt */
        printf("%s", PROMPT);
        fflush(stdout);

        /* Read line */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /* Skip empty lines */
        if (line[0] == '\0') {
            continue;
        }

        /* Check for exit */
        if (strcmp(line, "exit") == 0) {
            break;
        }

        /* Parse the line */
        ast = psInput(line);
        
        if (ast == NULL) {
            fprintf(stderr, "Parse error\n");
            continue;
        }

        /* Show the AST (for debugging) */
        printf("[AST] %s\n", showInput(ast));

        /* Execute */
        execute_input(ast);

        /* Free AST */
        free_Input(ast);
    }

    return EXIT_OK;
}
```

### 3.4 Update Makefile

Add to your main `Makefile`:

```makefile
# BNFC shell integration
BNFC_DIR = bnfc_shell
BNFC_SOURCES = $(BNFC_DIR)/Absyn.c $(BNFC_DIR)/Parser.c \
               $(BNFC_DIR)/Lexer.c $(BNFC_DIR)/Printer.c

SOURCES += shell_bnfc.c

# Build BNFC files first
.PHONY: bnfc
bnfc:
	cd $(BNFC_DIR) && bnfc --c Shell.cf && make

picobox: bnfc $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(BNFC_SOURCES)
```

### 3.5 Add Shell Mode Toggle

In `main.c`, add option to use BNFC shell:

```c
// Add prototype
int shell_bnfc_main(void);

// In main(), where you check for shell mode:
if (strcmp(program_name, "picobox") == 0) {
    if (argc < 2) {
        // Check environment variable for shell mode
        char *shell_mode = getenv("PICOBOX_BNFC");
        if (shell_mode && strcmp(shell_mode, "1") == 0) {
            return shell_bnfc_main();  // BNFC shell
        } else {
            return shell_main();        // Old shell
        }
    }
    // ... rest of code
}
```

### ✅ Phase 3 Verification Tests

**Test 3.1: Build system**
```bash
make clean
make
# Expected: Compiles successfully with BNFC integration
```

**Test 3.2: Parse simple command**
```bash
echo "cat" | PICOBOX_BNFC=1 ./picobox
# Expected: Shows AST for SimpleCmd("cat", [])
```

**Test 3.3: Parse command with arguments**
```bash
echo "cat file.txt" | PICOBOX_BNFC=1 ./picobox
# Expected: Shows AST for SimpleCmd("cat", ["file.txt"])
```

**Test 3.4: Parse multiple commands**
```bash
echo "ls ; pwd" | PICOBOX_BNFC=1 ./picobox
# Expected: Shows AST with two commands
```

**Test 3.5: Parse error handling**
```bash
echo "cat |" | PICOBOX_BNFC=1 ./picobox
# Expected: Parse error message (pipe not supported yet)
```

**📊 Phase 3 Checklist:**
- [ ] Shell.cf grammar created
- [ ] BNFC generates shell parser successfully
- [ ] shell_bnfc.c integrates with picobox
- [ ] Makefile builds everything correctly
- [ ] Can toggle between old/new shell
- [ ] All verification tests pass
- [ ] AST structures are understood

---

## PHASE 4: Fork/Exec Implementation

**Goal:** Replace in-process execution with proper fork/exec  
**Time Estimate:** 4-5 hours  
**Success Criteria:** Commands run in separate processes

### 4.1 Understand Current vs. Target Model

**Current (WRONG):**
```c
// In execute_command()
cmd_func = find_command("cat");
cmd_func(argc, argv);  // Runs cat() in SAME PROCESS
```

**Target (CORRECT):**
```c
// Fork a child process
pid_t pid = fork();
if (pid == 0) {
    // CHILD: execute command
    execvp("cat", argv);
    _exit(127);
}
// PARENT: wait for child
waitpid(pid, &status, 0);
```

### 4.2 Implement Fork/Exec Helper

Create `exec_helpers.c`:

```c
/*
 * exec_helpers.c - Process execution helpers
 */

#include "picobox.h"
#include <sys/wait.h>

/*
 * Execute command in child process
 * Returns exit status of child, or -1 on fork error
 */
int exec_command_external(char **argv)
{
    pid_t pid;
    int status;
    
    if (!argv || !argv[0]) {
        fprintf(stderr, "exec: null command\n");
        return EXIT_ERROR;
    }
    
    printf("[DEBUG] Forking to execute: %s\n", argv[0]);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return EXIT_ERROR;
    }
    
    if (pid == 0) {
        /* CHILD PROCESS */
        printf("[DEBUG-CHILD] Executing %s\n", argv[0]);
        
        execvp(argv[0], argv);
        
        /* If execvp returns, it failed */
        perror(argv[0]);
        _exit(127);
    }
    
    /* PARENT PROCESS */
    printf("[DEBUG-PARENT] Waiting for child %d\n", pid);
    
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_ERROR;
    }
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("[DEBUG-PARENT] Child exited with %d\n", exit_code);
        return exit_code;
    }
    
    if (WIFSIGNALED(status)) {
        printf("[DEBUG-PARENT] Child killed by signal %d\n", WTERMSIG(status));
        return 128 + WTERMSIG(status);
    }
    
    return EXIT_ERROR;
}

/*
 * Check if command is a built-in
 */
int is_builtin(const char *cmd)
{
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "help") == 0);
}

/*
 * Execute built-in command
 */
int exec_builtin(int argc, char **argv)
{
    if (strcmp(argv[0], "cd") == 0) {
        // Call existing builtin_cd from shell.c
        // For now, implement inline
        if (argc < 2) {
            fprintf(stderr, "cd: missing argument\n");
            return EXIT_ERROR;
        }
        if (chdir(argv[1]) != 0) {
            perror("cd");
            return EXIT_ERROR;
        }
        return EXIT_OK;
    }
    
    if (strcmp(argv[0], "exit") == 0) {
        return -1;  // Signal to exit shell
    }
    
    fprintf(stderr, "Unknown builtin: %s\n", argv[0]);
    return EXIT_ERROR;
}
```

### 4.3 Update Shell BNFC to Use Fork/Exec

Modify `shell_bnfc.c`:

```c
// Add include
#include "exec_helpers.h"

/*
 * Convert BNFC Word list to argv array
 */
static char **words_to_argv(Word *cmd_word, ListWord *arg_words, int *argc_out)
{
    // Count total words
    int count = 1;  // Command itself
    ListWord *w = arg_words;
    while (w) {
        count++;
        w = w->listword_;
    }
    
    // Allocate argv
    char **argv = malloc((count + 1) * sizeof(char *));
    if (!argv) {
        return NULL;
    }
    
    // Fill argv
    argv[0] = strdup(cmd_word);
    int i = 1;
    w = arg_words;
    while (w && w->word_) {
        argv[i++] = strdup(w->word_);
        w = w->listword_;
    }
    argv[i] = NULL;
    
    *argc_out = count;
    return argv;
}

/*
 * Free argv array
 */
static void free_argv(char **argv)
{
    if (!argv) return;
    for (int i = 0; argv[i]; i++) {
        free(argv[i]);
    }
    free(argv);
}

/*
 * Execute a simple command using fork/exec
 */
static int execute_simple_command(Command *cmd)
{
    char **argv;
    int argc;
    int status;
    
    // Convert BNFC structures to argv
    argv = words_to_argv(cmd->u.simplecmd_.word_,
                         cmd->u.simplecmd_.listword_,
                         &argc);
    
    if (!argv) {
        fprintf(stderr, "Failed to build argv\n");
        return EXIT_ERROR;
    }
    
    printf("[DEBUG] Executing: %s", argv[0]);
    for (int i = 1; i < argc; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
    
    // Check if builtin
    if (is_builtin(argv[0])) {
        status = exec_builtin(argc, argv);
    } else {
        // External command - fork/exec
        status = exec_command_external(argv);
    }
    
    free_argv(argv);
    return status;
}
```

### 4.4 Create Test Files

Create `tests/cases/04_fork_exec_basic.txt`:
```bash
echo "Hello World"
```

Create `tests/expected/04_fork_exec_basic.txt`:
```
Hello World
```

Create `tests/cases/04_fork_exec_pwd.txt`:
```bash
pwd
```

Create `tests/cases/04_fork_exec_multiple.txt`:
```bash
echo "First" ; echo "Second"
```

### 4.5 Create Test Runner

Create `tests/run_tests.sh`:

```bash
#!/bin/bash

PICOBOX="../picobox"
CASES_DIR="cases"
EXPECTED_DIR="expected"

PASS=0
FAIL=0

export PICOBOX_BNFC=1

for test_case in "$CASES_DIR"/*.txt; do
    test_name=$(basename "$test_case" .txt)
    expected="$EXPECTED_DIR/$test_name.txt"
    
    echo "Running test: $test_name"
    
    if [ ! -f "$expected" ]; then
        echo "  ⚠️  No expected output file, skipping"
        continue
    fi
    
    # Run test
    output=$(cat "$test_case" | $PICOBOX 2>&1)
    expected_output=$(cat "$expected")
    
    if [ "$output" = "$expected_output" ]; then
        echo "  ✅ PASS"
        ((PASS++))
    else
        echo "  ❌ FAIL"
        echo "  Expected: $expected_output"
        echo "  Got:      $output"
        ((FAIL++))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed"

if [ $FAIL -eq 0 ]; then
    exit 0
else
    exit 1
fi
```

```bash
chmod +x tests/run_tests.sh
```

### ✅ Phase 4 Verification Tests

**Test 4.1: Fork echo command**
```bash
echo "echo Hello" | PICOBOX_BNFC=1 ./picobox
# Expected: Prints "Hello" (from child process)
# Expected: Debug messages show fork/exec happening
```

**Test 4.2: Fork with arguments**
```bash
echo "echo Hello World" | PICOBOX_BNFC=1 ./picobox
# Expected: Prints "Hello World"
```

**Test 4.3: Multiple commands**
```bash
echo "echo First ; echo Second" | PICOBOX_BNFC=1 ./picobox
# Expected: Prints both "First" and "Second"
```

**Test 4.4: Builtin cd still works**
```bash
echo "cd /tmp ; pwd" | PICOBOX_BNFC=1 ./picobox
# Expected: pwd shows /tmp
```

**Test 4.5: Error handling - bad command**
```bash
echo "nonexistent_command" | PICOBOX_BNFC=1 ./picobox
# Expected: Error message, shell continues
```

**Test 4.6: Run automated tests**
```bash
cd tests
./run_tests.sh
# Expected: All Phase 4 tests pass
```

**📊 Phase 4 Checklist:**
- [ ] exec_helpers.c created and compiles
- [ ] Fork/exec implemented correctly
- [ ] Built-ins still work (cd, exit)
- [ ] External commands work (echo, ls, cat)
- [ ] Process isolation verified (errors don't crash shell)
- [ ] All verification tests pass
- [ ] Test suite created and passing

---

## PHASE 5: Pipes Implementation

**Goal:** Support pipeline execution (cmd1 | cmd2 | cmd3)  
**Time Estimate:** 5-6 hours  
**Success Criteria:** Can pipe data between commands

### 5.1 Update Grammar for Pipes

Modify `Shell.cf`:

```bnfc
-- Shell grammar - Phase 5 (add pipes)
comment "/*" "*/" ;
comment "//" "\n" ;

token Word (letter (letter | digit | [._/\-])*) ;

entry Input ;

StartInput. Input ::= [Command] ;

-- Now commands can be pipelines
CmdPipeline. Command ::= Pipeline ;

-- Pipeline is one or more simple commands
SingleCmd. Pipeline ::= SimpleCmd ;
PipeCmd.   Pipeline ::= SimpleCmd "|" Pipeline ;

-- Simple command is word + args
Cmd. SimpleCmd ::= Word [Word] ;

separator Command ";" ;
separator Word " " ;
```

### 5.2 Regenerate Parser

```bash
cd bnfc_shell/
bnfc --c Shell.cf
make clean
make
```

### 5.3 Implement Pipeline Execution

Add to `exec_helpers.c`:

```c
/*
 * Execute a pipeline of commands
 * 
 * Pipeline: cmd1 | cmd2 | cmd3
 * 
 * Creates pipes:
 *   pipe0: cmd1 -> cmd2
 *   pipe1: cmd2 -> cmd3
 * 
 * Wiring:
 *   cmd1: stdout -> pipe0[write]
 *   cmd2: stdin <- pipe0[read], stdout -> pipe1[write]
 *   cmd3: stdin <- pipe1[read]
 */
int exec_pipeline(char ***cmds, int num_cmds)
{
    int i;
    int num_pipes = num_cmds - 1;
    int pipes[num_pipes][2];
    pid_t pids[num_cmds];
    int status;
    int final_status = 0;
    
    printf("[DEBUG] Executing pipeline with %d commands\n", num_cmds);
    
    /* Create all pipes */
    for (i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return EXIT_ERROR;
        }
        printf("[DEBUG] Created pipe %d: [%d, %d]\n", 
               i, pipes[i][0], pipes[i][1]);
    }
    
    /* Fork and execute each command */
    for (i = 0; i < num_cmds; i++) {
        printf("[DEBUG] Forking command %d: %s\n", i, cmds[i][0]);
        
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            return EXIT_ERROR;
        }
        
        if (pids[i] == 0) {
            /* CHILD PROCESS */
            
            /* Wire input (from previous pipe) */
            if (i > 0) {
                printf("[DEBUG-CHILD-%d] Wiring stdin from pipe %d\n", 
                       i, i-1);
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            /* Wire output (to next pipe) */
            if (i < num_cmds - 1) {
                printf("[DEBUG-CHILD-%d] Wiring stdout to pipe %d\n", 
                       i, i);
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            /* Close ALL pipe file descriptors in child */
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            /* Execute command */
            printf("[DEBUG-CHILD-%d] Executing %s\n", i, cmds[i][0]);
            execvp(cmds[i][0], cmds[i]);
            
            /* If execvp returns, it failed */
            perror(cmds[i][0]);
            _exit(127);
        }
    }
    
    /* PARENT: Close all pipes */
    for (i = 0; i < num_pipes; i++) {
        printf("[DEBUG-PARENT] Closing pipe %d\n", i);
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    /* Wait for all children */
    for (i = 0; i < num_cmds; i++) {
        printf("[DEBUG-PARENT] Waiting for child %d (pid %d)\n", 
               i, pids[i]);
        
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            final_status = EXIT_ERROR;
            continue;
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("[DEBUG-PARENT] Child %d exited with %d\n", 
                   i, exit_code);
            
            /* Use last command's exit status */
            if (i == num_cmds - 1) {
                final_status = exit_code;
            }
        }
    }
    
    return final_status;
}
```

### 5.4 Update Shell BNFC for Pipelines

Modify `shell_bnfc.c`:

```c
/*
 * Count pipeline stages
 */
static int count_pipeline_stages(Pipeline *p)
{
    int count = 0;
    while (p) {
        count++;
        if (p->kind == is_SingleCmd) {
            break;
        }
        // PipeCmd has a continuation
        p = p->u.pipecmd_.pipeline_;
    }
    return count;
}

/*
 * Build array of commands from pipeline
 */
static char ***build_pipeline_cmds(Pipeline *p, int *num_cmds_out)
{
    int num_cmds = count_pipeline_stages(p);
    char ***cmds = malloc(num_cmds * sizeof(char **));
    
    int idx = 0;
    Pipeline *curr = p;
    
    while (curr) {
        SimpleCmd *sc;
        
        if (curr->kind == is_SingleCmd) {
            sc = curr->u.singlecmd_.simplecmd_;
        } else {
            sc = curr->u.pipecmd_.simplecmd_;
        }
        
        // Convert SimpleCmd to argv
        int argc;
        cmds[idx] = words_to_argv(
            sc->u.cmd_.word_,
            sc->u.cmd_.listword_,
            &argc
        );
        
        idx++;
        
        if (curr->kind == is_SingleCmd) {
            break;
        }
        
        curr = curr->u.pipecmd_.pipeline_;
    }
    
    *num_cmds_out = num_cmds;
    return cmds;
}

/*
 * Execute a command (which might be a pipeline)
 */
static int execute_command_node(Command *cmd)
{
    if (cmd->kind != is_CmdPipeline) {
        fprintf(stderr, "Unknown command type\n");
        return EXIT_ERROR;
    }
    
    Pipeline *p = cmd->u.cmdpipeline_.pipeline_;
    
    // Check if single command or actual pipeline
    if (p->kind == is_SingleCmd) {
        // Single command - use simple exec
        SimpleCmd *sc = p->u.singlecmd_.simplecmd_;
        int argc;
        char **argv = words_to_argv(
            sc->u.cmd_.word_,
            sc->u.cmd_.listword_,
            &argc
        );
        
        int status;
        if (is_builtin(argv[0])) {
            status = exec_builtin(argc, argv);
        } else {
            status = exec_command_external(argv);
        }
        
        free_argv(argv);
        return status;
        
    } else {
        // Actual pipeline
        int num_cmds;
        char ***cmds = build_pipeline_cmds(p, &num_cmds);
        
        int status = exec_pipeline(cmds, num_cmds);
        
        // Free cmds
        for (int i = 0; i < num_cmds; i++) {
            free_argv(cmds[i]);
        }
        free(cmds);
        
        return status;
    }
}
```

### 5.5 Create Pipeline Test Cases

Create `tests/cases/05_pipe_simple.txt`:
```bash
echo "hello world" | cat
```

Expected: `hello world`

Create `tests/cases/05_pipe_grep.txt`:
```bash
echo -e "line1\nline2\nline3" | grep line2
```

Expected: `line2`

Create `tests/cases/05_pipe_three.txt`:
```bash
echo "HELLO" | tr A-Z a-z | cat
```

Expected: `hello`

Create `tests/cases/05_pipe_wc.txt`:
```bash
echo -e "a\nb\nc" | wc -l
```

Expected: `3`

### ✅ Phase 5 Verification Tests

**Test 5.1: Simple pipe**
```bash
echo 'echo "test" | cat' | PICOBOX_BNFC=1 ./picobox
# Expected: "test"
```

**Test 5.2: Pipe with grep**
```bash
echo 'echo -e "foo\nbar\nbaz" | grep bar' | PICOBOX_BNFC=1 ./picobox
# Expected: "bar"
```

**Test 5.3: Three-stage pipeline**
```bash
echo 'echo "HELLO" | tr A-Z a-z | cat' | PICOBOX_BNFC=1 ./picobox
# Expected: "hello"
```

**Test 5.4: Pipeline with wc**
```bash
echo 'ls | wc -l' | PICOBOX_BNFC=1 ./picobox
# Expected: Number of files in current directory
```

**Test 5.5: Multiple pipelines**
```bash
echo 'echo A | cat ; echo B | cat' | PICOBOX_BNFC=1 ./picobox
# Expected: "A" then "B"
```

**Test 5.6: Automated test suite**
```bash
cd tests
./run_tests.sh
# Expected: All Phase 5 tests pass
```

**📊 Phase 5 Checklist:**
- [ ] Grammar updated for pipes
- [ ] Parser regenerated successfully
- [ ] Pipeline execution implemented
- [ ] Pipe file descriptors managed correctly
- [ ] All children waited for properly
- [ ] All verification tests pass
- [ ] Test suite expanded and passing

---

## PHASE 6: Redirections Implementation

**Goal:** Support I/O redirections (<, >, >>)  
**Time Estimate:** 4-5 hours  
**Success Criteria:** Can redirect input/output to files

### 6.1 Update Grammar for Redirections

Modify `Shell.cf`:

```bnfc
-- Shell grammar - Phase 6 (add redirections)
comment "/*" "*/" ;
comment "//" "\n" ;

token Word (letter (letter | digit | [._/\-])*) ;

entry Input ;

StartInput. Input ::= [Command] ;

-- Commands with optional redirections
CmdRedir. Command ::= Pipeline [Redir] ;

-- Redirections
RedirIn.     Redir ::= "<" Word ;
RedirOut:    Redir ::= ">" Word ;
RedirAppend. Redir ::= ">>" Word ;

-- Pipelines
SingleCmd. Pipeline ::= SimpleCmd ;
PipeCmd.   Pipeline ::= SimpleCmd "|" Pipeline ;

-- Simple commands
Cmd. SimpleCmd ::= Word [Word] ;

separator Command ";" ;
separator Redir "" ;
separator Word " " ;
```

### 6.2 Regenerate Parser

```bash
cd bnfc_shell/
bnfc --c Shell.cf
make clean
make
```

### 6.3 Implement Redirection Helpers

Add to `exec_helpers.c`:

```c
/*
 * Redirection structure (internal)
 */
typedef struct {
    int type;           // 0=in, 1=out, 2=append
    char *filename;
} redir_t;

/*
 * Apply redirections in current process
 * Call this BEFORE exec in child process
 */
int apply_redirections(redir_t *redirs, int num_redirs)
{
    for (int i = 0; i < num_redirs; i++) {
        int fd;
        
        switch (redirs[i].type) {
        case 0:  /* Input redirection < */
            printf("[DEBUG] Redirecting stdin from %s\n", 
                   redirs[i].filename);
            fd = open(redirs[i].filename, O_RDONLY);
            if (fd < 0) {
                perror(redirs[i].filename);
                return -1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            break;
            
        case 1:  /* Output redirection > */
            printf("[DEBUG] Redirecting stdout to %s\n", 
                   redirs[i].filename);
            fd = open(redirs[i].filename, 
                     O_WRONLY | O_CREAT | O_TRUNC, 
                     0644);
            if (fd < 0) {
                perror(redirs[i].filename);
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            break;
            
        case 2:  /* Append redirection >> */
            printf("[DEBUG] Appending stdout to %s\n", 
                   redirs[i].filename);
            fd = open(redirs[i].filename, 
                     O_WRONLY | O_CREAT | O_APPEND, 
                     0644);
            if (fd < 0) {
                perror(redirs[i].filename);
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            break;
        }
    }
    
    return 0;
}

/*
 * Execute command with redirections
 */
int exec_command_with_redir(char **argv, redir_t *redirs, int num_redirs)
{
    pid_t pid;
    int status;
    
    printf("[DEBUG] Executing with %d redirections: %s\n", 
           num_redirs, argv[0]);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return EXIT_ERROR;
    }
    
    if (pid == 0) {
        /* CHILD */
        
        /* Apply redirections */
        if (apply_redirections(redirs, num_redirs) < 0) {
            _exit(EXIT_ERROR);
        }
        
        /* Execute */
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }
    
    /* PARENT */
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_ERROR;
    }
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return EXIT_ERROR;
}

/*
 * Execute pipeline with final redirections
 * Redirections apply to the LAST command in the pipeline
 */
int exec_pipeline_with_redir(char ***cmds, int num_cmds, 
                              redir_t *redirs, int num_redirs)
{
    int i;
    int num_pipes = num_cmds - 1;
    int pipes[num_pipes][2];
    pid_t pids[num_cmds];
    int status;
    int final_status = 0;
    
    /* Create pipes */
    for (i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return EXIT_ERROR;
        }
    }
    
    /* Fork each command */
    for (i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            return EXIT_ERROR;
        }
        
        if (pids[i] == 0) {
            /* CHILD */
            
            /* Wire pipes */
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            /* Close all pipe FDs */
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            /* Apply redirections (only on last command) */
            if (i == num_cmds - 1) {
                if (apply_redirections(redirs, num_redirs) < 0) {
                    _exit(EXIT_ERROR);
                }
            }
            
            /* Execute */
            execvp(cmds[i][0], cmds[i]);
            perror(cmds[i][0]);
            _exit(127);
        }
    }
    
    /* PARENT: Close pipes and wait */
    for (i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    for (i = 0; i < num_cmds; i++) {
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            final_status = EXIT_ERROR;
            continue;
        }
        
        if (WIFEXITED(status) && i == num_cmds - 1) {
            final_status = WEXITSTATUS(status);
        }
    }
    
    return final_status;
}
```

### 6.4 Update Shell BNFC for Redirections

Modify `shell_bnfc.c`:

```c
/*
 * Convert BNFC redirections to internal format
 */
static redir_t *build_redirections(ListRedir *list, int *num_out)
{
    // Count redirections
    int count = 0;
    ListRedir *l = list;
    while (l && l->redir_) {
        count++;
        l = l->listredir_;
    }
    
    if (count == 0) {
        *num_out = 0;
        return NULL;
    }
    
    // Allocate array
    redir_t *redirs = malloc(count * sizeof(redir_t));
    
    // Fill array
    int idx = 0;
    l = list;
    while (l && l->redir_) {
        Redir *r = l->redir_;
        
        switch (r->kind) {
        case is_RedirIn:
            redirs[idx].type = 0;
            redirs[idx].filename = strdup(r->u.redirin_.word_);
            break;
            
        case is_RedirOut:
            redirs[idx].type = 1;
            redirs[idx].filename = strdup(r->u.redirout_.word_);
            break;
            
        case is_RedirAppend:
            redirs[idx].type = 2;
            redirs[idx].filename = strdup(r->u.redirappend_.word_);
            break;
        }
        
        idx++;
        l = l->listredir_;
    }
    
    *num_out = count;
    return redirs;
}

/*
 * Execute command with redirections
 */
static int execute_command_node(Command *cmd)
{
    if (cmd->kind != is_CmdRedir) {
        fprintf(stderr, "Unknown command type\n");
        return EXIT_ERROR;
    }
    
    Pipeline *p = cmd->u.cmdredir_.pipeline_;
    ListRedir *rlist = cmd->u.cmdredir_.listredir_;
    
    // Build redirections
    int num_redirs;
    redir_t *redirs = build_redirections(rlist, &num_redirs);
    
    int status;
    
    if (p->kind == is_SingleCmd) {
        // Single command
        SimpleCmd *sc = p->u.singlecmd_.simplecmd_;
        int argc;
        char **argv = words_to_argv(
            sc->u.cmd_.word_,
            sc->u.cmd_.listword_,
            &argc
        );
        
        if (num_redirs > 0) {
            status = exec_command_with_redir(argv, redirs, num_redirs);
        } else {
            if (is_builtin(argv[0])) {
                status = exec_builtin(argc, argv);
            } else {
                status = exec_command_external(argv);
            }
        }
        
        free_argv(argv);
        
    } else {
        // Pipeline
        int num_cmds;
        char ***cmds = build_pipeline_cmds(p, &num_cmds);
        
        if (num_redirs > 0) {
            status = exec_pipeline_with_redir(cmds, num_cmds, 
                                             redirs, num_redirs);
        } else {
            status = exec_pipeline(cmds, num_cmds);
        }
        
        for (int i = 0; i < num_cmds; i++) {
            free_argv(cmds[i]);
        }
        free(cmds);
    }
    
    // Free redirections
    if (redirs) {
        for (int i = 0; i < num_redirs; i++) {
            free(redirs[i].filename);
        }
        free(redirs);
    }
    
    return status;
}
```

### 6.5 Create Redirection Test Cases

Create `tests/cases/06_redir_out.txt`:
```bash
echo "test output" > /tmp/test.txt
cat /tmp/test.txt
```

Expected: `test output`

Create `tests/cases/06_redir_in.txt`:
```bash
echo "input data" > /tmp/input.txt
cat < /tmp/input.txt
```

Expected: `input data`

Create `tests/cases/06_redir_append.txt`:
```bash
echo "line1" > /tmp/append.txt
echo "line2" >> /tmp/append.txt
cat /tmp/append.txt
```

Expected:
```
line1
line2
```

Create `tests/cases/06_redir_pipe_out.txt`:
```bash
echo "HELLO" | tr A-Z a-z > /tmp/lower.txt
cat /tmp/lower.txt
```

Expected: `hello`

### ✅ Phase 6 Verification Tests

**Test 6.1: Output redirection**
```bash
rm -f /tmp/test_out.txt
echo 'echo "hello" > /tmp/test_out.txt' | PICOBOX_BNFC=1 ./picobox
cat /tmp/test_out.txt
# Expected: "hello"
```

**Test 6.2: Input redirection**
```bash
echo "test data" > /tmp/test_in.txt
echo 'cat < /tmp/test_in.txt' | PICOBOX_BNFC=1 ./picobox
# Expected: "test data"
```

**Test 6.3: Append redirection**
```bash
rm -f /tmp/test_append.txt
echo 'echo "A" > /tmp/test_append.txt' | PICOBOX_BNFC=1 ./picobox
echo 'echo "B" >> /tmp/test_append.txt' | PICOBOX_BNFC=1 ./picobox
cat /tmp/test_append.txt
# Expected: "A" then "B"
```

**Test 6.4: Pipeline with output redirection**
```bash
rm -f /tmp/test_pipe_out.txt
echo 'echo "HELLO" | tr A-Z a-z > /tmp/test_pipe_out.txt' | PICOBOX_BNFC=1 ./picobox
cat /tmp/test_pipe_out.txt
# Expected: "hello"
```

**Test 6.5: Error handling - bad file**
```bash
echo 'cat < /nonexistent/file.txt' | PICOBOX_BNFC=1 ./picobox
# Expected: Error message about file not found
```

**Test 6.6: Automated test suite**
```bash
cd tests
./run_tests.sh
# Expected: All Phase 6 tests pass
```

**📊 Phase 6 Checklist:**
- [ ] Grammar updated for redirections
- [ ] Parser regenerated successfully
- [ ] Redirection logic implemented correctly
- [ ] File descriptors managed properly
- [ ] Works with both simple commands and pipelines
- [ ] All verification tests pass
- [ ] Test suite expanded and passing

---

## PHASE 7: Background Jobs

**Goal:** Support background execution (cmd &)  
**Time Estimate:** 3-4 hours  
**Success Criteria:** Can run jobs in background

### 7.1 Update Grammar for Background

Modify `Shell.cf`:

```bnfc
-- Shell grammar - Phase 7 (add background jobs)
comment "/*" "*/" ;
comment "//" "\n" ;

token Word (letter (letter | digit | [._/\-])*) ;

entry Input ;

StartInput. Input ::= [Job] ;

-- Jobs can be foreground or background
JobFG. Job ::= Command ;
JobBG. Job ::= Command "&" ;

-- Commands with redirections
CmdRedir. Command ::= Pipeline [Redir] ;

-- Redirections
RedirIn.     Redir ::= "<" Word ;
RedirOut.    Redir ::= ">" Word ;
RedirAppend. Redir ::= ">>" Word ;

-- Pipelines
SingleCmd. Pipeline ::= SimpleCmd ;
PipeCmd.   Pipeline ::= SimpleCmd "|" Pipeline ;

-- Simple commands
Cmd. SimpleCmd ::= Word [Word] ;

separator Job ";" ;
separator Redir "" ;
separator Word " " ;
```

### 7.2 Implement Background Job Support

Add to `exec_helpers.c`:

```c
#include <signal.h>

/* Job tracking */
typedef struct {
    int job_num;
    pid_t pid;
    char *command;
    int running;
} job_t;

#define MAX_JOBS 100
static job_t jobs[MAX_JOBS];
static int num_jobs = 0;
static int next_job_num = 1;

/*
 * SIGCHLD handler - reaps background jobs
 */
static void sigchld_handler(int sig)
{
    int status;
    pid_t pid;
    
    (void)sig;  // Unused
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Find the job
        for (int i = 0; i < num_jobs; i++) {
            if (jobs[i].pid == pid && jobs[i].running) {
                jobs[i].running = 0;
                
                printf("\n[%d] Done    %s\n", 
                       jobs[i].job_num, 
                       jobs[i].command);
                fflush(stdout);
                break;
            }
        }
    }
}

/*
 * Initialize job control (call once at shell startup)
 */
void init_job_control(void)
{
    struct sigaction sa;
    
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction");
    }
}

/*
 * Add a background job
 */
static void add_job(pid_t pid, const char *cmdline)
{
    if (num_jobs >= MAX_JOBS) {
        fprintf(stderr, "Too many background jobs\n");
        return;
    }
    
    jobs[num_jobs].job_num = next_job_num++;
    jobs[num_jobs].pid = pid;
    jobs[num_jobs].command = strdup(cmdline);
    jobs[num_jobs].running = 1;
    
    printf("[%d] %d\n", jobs[num_jobs].job_num, pid);
    
    num_jobs++;
}

/*
 * List active jobs
 */
void list_jobs(void)
{
    for (int i = 0; i < num_jobs; i++) {
        if (jobs[i].running) {
            printf("[%d] Running    %s\n", 
                   jobs[i].job_num, 
                   jobs[i].command);
        }
    }
}

/*
 * Execute command in background
 * Returns immediately, doesn't wait for completion
 */
int exec_command_background(char **argv, const char *cmdline)
{
    pid_t pid;
    
    printf("[DEBUG] Starting background job: %s\n", argv[0]);
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return EXIT_ERROR;
    }
    
    if (pid == 0) {
        /* CHILD */
        
        /* Ignore SIGINT in background jobs */
        signal(SIGINT, SIG_IGN);
        
        /* Execute */
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);
    }
    
    /* PARENT - don't wait, just track */
    add_job(pid, cmdline);
    
    return EXIT_OK;
}

/*
 * Execute pipeline in background
 */
int exec_pipeline_background(char ***cmds, int num_cmds, const char *cmdline)
{
    int i;
    int num_pipes = num_cmds - 1;
    int pipes[num_pipes][2];
    pid_t main_pid;
    
    /* Create pipes */
    for (i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return EXIT_ERROR;
        }
    }
    
    /* Fork main process for the pipeline */
    main_pid = fork();
    
    if (main_pid < 0) {
        perror("fork");
        return EXIT_ERROR;
    }
    
    if (main_pid == 0) {
        /* CHILD - fork pipeline stages */
        
        for (i = 0; i < num_cmds; i++) {
            pid_t pid = fork();
            
            if (pid == 0) {
                /* Pipeline stage child */
                
                if (i > 0) {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                if (i < num_cmds - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                
                for (int j = 0; j < num_pipes; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                
                execvp(cmds[i][0], cmds[i]);
                perror(cmds[i][0]);
                _exit(127);
            }
        }
        
        /* Pipeline parent - close pipes and wait */
        for (i = 0; i < num_pipes; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        
        for (i = 0; i < num_cmds; i++) {
            wait(NULL);
        }
        
        _exit(EXIT_OK);
    }
    
    /* Main shell process - close pipes and track job */
    for (i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    add_job(main_pid, cmdline);
    
    return EXIT_OK;
}
```

### 7.3 Update Shell BNFC for Background Jobs

Modify `shell_bnfc.c`:

```c
// Add at startup
int shell_bnfc_main(void)
{
    // ... existing code ...
    
    /* Initialize job control */
    init_job_control();
    
    // ... rest of shell loop ...
}

/*
 * Execute a job (might be background)
 */
static int execute_job(Job *job, const char *cmdline)
{
    int background = 0;
    Command *cmd;
    
    if (job->kind == is_JobBG) {
        background = 1;
        cmd = job->u.jobbg_.command_;
    } else {
        cmd = job->u.jobfg_.command_;
    }
    
    // ... get pipeline and redirections as before ...
    
    int status;
    
    if (background) {
        printf("[DEBUG] Running in background\n");
        
        if (is_pipeline) {
            status = exec_pipeline_background(cmds, num_cmds, cmdline);
        } else {
            status = exec_command_background(argv, cmdline);
        }
    } else {
        // Foreground execution as before
        // ...
    }
    
    return status;
}
```

### 7.4 Add Built-in Jobs Command

```c
// In builtin handling
if (strcmp(argv[0], "jobs") == 0) {
    list_jobs();
    return EXIT_OK;
}
```

### 7.5 Create Background Job Test Cases

Create `tests/cases/07_bg_simple.txt`:
```bash
sleep 2 &
echo "Immediate"
```

Expected: Job number printed, then "Immediate" immediately

Create `tests/cases/07_bg_multiple.txt`:
```bash
sleep 1 &
sleep 1 &
jobs
```

Expected: Two job numbers, then job list showing both running

### ✅ Phase 7 Verification Tests

**Test 7.1: Simple background job**
```bash
echo 'sleep 2 & ; echo "Done"' | PICOBOX_BNFC=1 ./picobox
# Expected: "[1] <pid>" then "Done" immediately
# Then after 2 seconds: "[1] Done    sleep 2"
```

**Test 7.2: Multiple background jobs**
```bash
echo 'sleep 3 & ; sleep 3 & ; jobs' | PICOBOX_BNFC=1 ./picobox
# Expected: Two job numbers, then job list
```

**Test 7.3: Background pipeline**
```bash
echo 'echo "test" | cat & ; echo "Done"' | PICOBOX_BNFC=1 ./picobox
# Expected: Job started, "Done" printed immediately
```

**Test 7.4: Foreground still works**
```bash
echo 'sleep 1 ; echo "After sleep"' | PICOBOX_BNFC=1 ./picobox
# Expected: Waits 1 second, then prints "After sleep"
```

**Test 7.5: Jobs command**
```bash
echo 'sleep 10 & ; jobs' | PICOBOX_BNFC=1 ./picobox
# Expected: Shows running job
```

**📊 Phase 7 Checklist:**
- [ ] Grammar updated for background jobs
- [ ] SIGCHLD handler implemented
- [ ] Job tracking works correctly
- [ ] Background jobs don't block shell
- [ ] jobs command works
- [ ] All verification tests pass

---

## PHASE 8: Integration & Polish

**Goal:** Clean up, document, and finalize  
**Time Estimate:** 2-3 hours  
**Success Criteria:** Production-ready shell

### 8.1 Remove Debug Output

Go through all files and either remove or make conditional:

```c
// Change this:
printf("[DEBUG] Forking...\n");

// To this:
#ifdef DEBUG_SHELL
printf("[DEBUG] Forking...\n");
#endif
```

### 8.2 Error Handling Audit

Ensure all system calls check errors:

```c
// Check all:
- fork()
- pipe()
- dup2()
- open()
- waitpid()
- execvp()
```

### 8.3 Memory Leak Check

```bash
# Run with valgrind
echo 'echo "test" ; exit' | valgrind --leak-check=full ./picobox
# Expected: No memory leaks
```

### 8.4 Create Comprehensive Test Suite

Create `tests/comprehensive.sh`:

```bash
#!/bin/bash

echo "=== Comprehensive Shell Test Suite ==="

TESTS_PASSED=0
TESTS_FAILED=0

# Test 1: Simple command
# Test 2: Pipes
# Test 3: Redirections
# Test 4: Background jobs
# Test 5: Error handling
# ... etc

echo ""
echo "Results: $TESTS_PASSED passed, $TESTS_FAILED failed"
```

### 8.5 Documentation

Create `README_BNFC.md`:

```markdown
# PicoBox BNFC Shell

## Features
- ✅ Command execution with fork/exec
- ✅ Pipelines (cmd1 | cmd2 | cmd3)
- ✅ Redirections (<, >, >>)
- ✅ Background jobs (cmd &)
- ✅ Built-in commands (cd, exit, help, jobs)

## Usage
```bash
# Enable BNFC shell
export PICOBOX_BNFC=1
./picobox

# Run tests
cd tests
./run_tests.sh
```

## Architecture
- Grammar: `bnfc_shell/Shell.cf`
- Parser: Generated by BNFC
- Execution: `shell_bnfc.c`, `exec_helpers.c`
```

### ✅ Phase 8 Verification

**Final test checklist:**
- [ ] All debug output removed or conditional
- [ ] No memory leaks (valgrind clean)
- [ ] All error paths tested
- [ ] Comprehensive test suite passes
- [ ] Documentation complete
- [ ] Code reviewed and clean

---

## TESTING STRATEGY

### Unit Testing (Per Phase)

Each phase MUST pass its verification tests before proceeding.

```bash
# After each phase:
1. Run phase-specific tests
2. Verify expected output
3. Check for errors
4. Only proceed if ALL tests pass
```

### Integration Testing

```bash
# Create test/integration/
# Tests that combine features:
- Pipes + redirections
- Background + pipes
- Multiple commands with various features
```

### Regression Testing

```bash
# After each phase, run ALL previous tests
# Ensure new features don't break old ones

./tests/run_all_tests.sh
```

### Error Testing

```bash
# Specific error condition tests:
- Bad command names
- Missing files
- Permission errors
- Syntax errors
- Resource exhaustion
```

### Test Organization

```
tests/
├── cases/
│   ├── 00_empty.txt
│   ├── 01_simple_echo.txt
│   ├── 02_simple_multiple.txt
│   ├── 03_grammar_parse.txt
│   ├── 04_fork_exec_basic.txt
│   ├── 05_pipe_simple.txt
│   ├── 06_redir_out.txt
│   ├── 07_bg_simple.txt
│   └── ...
├── expected/
│   ├── 00_empty.txt
│   ├── 01_simple_echo.txt
│   └── ...
├── run_tests.sh
├── integration/
│   ├── pipe_redir.sh
│   └── ...
└── errors/
    ├── bad_command.sh
    └── ...
```

---

## SUCCESS CRITERIA SUMMARY

### Phase 1: Prerequisites ✓
- [ ] BNFC installed
- [ ] All tools verified
- [ ] Directory structure ready

### Phase 2: Calculator Learning ✓
- [ ] Understands BNFC workflow
- [ ] Can modify and test calculator
- [ ] Understands visitor pattern

### Phase 3: Grammar Integration ✓
- [ ] Simple grammar works
- [ ] Parser generates successfully
- [ ] Can parse basic commands

### Phase 4: Fork/Exec ✓
- [ ] Commands run in separate processes
- [ ] Built-ins still work
- [ ] Error handling robust

### Phase 5: Pipes ✓
- [ ] Can pipe between 2+ commands
- [ ] File descriptors managed correctly
- [ ] All children waited for

### Phase 6: Redirections ✓
- [ ] Input/output redirection works
- [ ] Append mode works
- [ ] Works with pipes

### Phase 7: Background Jobs ✓
- [ ] Background execution works
- [ ] SIGCHLD handling correct
- [ ] Job tracking works

### Phase 8: Polish ✓
- [ ] No memory leaks
- [ ] Clean code
- [ ] Full documentation

---

## CRITICAL REMINDERS

1. **TEST AFTER EVERY CHANGE**
   - Don't write 100 lines then test
   - Write 10 lines, test, repeat

2. **ONE FEATURE AT A TIME**
   - Don't mix phases
   - Complete verification before moving on

3. **SAVE WORKING STATES**
   - Git commit after each passing phase
   - Tag versions: v1-prereq, v2-calc, etc.

4. **DEBUG THOROUGHLY**
   - Use printf debugging
   - Check all error paths
   - Verify with different inputs

5. **ASK FOR HELP**
   - If stuck for >30 minutes on same issue
   - Check professor's example files
   - Use AI agent with specific questions

---

## ESTIMATED TIMELINE

- **Week 1:** Phases 1-3 (Setup + Grammar)
- **Week 2:** Phases 4-5 (Exec + Pipes)
- **Week 3:** Phases 6-7 (Redir + Background)
- **Week 4:** Phase 8 (Polish + Documentation)

**Total:** ~4 weeks at 10-15 hours/week

---

## END OF PLAN

This plan provides a complete roadmap from current state to fully functional BNFC-powered shell with pipes, redirections, and background jobs. Follow it incrementally, test continuously, and you WILL succeed! 🚀