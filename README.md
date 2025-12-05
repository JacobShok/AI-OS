# PicoBox - Technical Documentation

## Overview

**PicoBox** is a BusyBox-style Unix utilities implementation written in C that provides a modern, AI-enhanced interactive shell environment. It combines traditional Unix command-line tools with advanced features like grammar-based parsing (BNFC), pipeline execution, I/O redirection, and dual AI assistant systems.

**Version:** 0.1.0
**Language:** C (C11 standard)
**Key Technologies:** BNFC (Parser Generator), OpenAI API, Argtable3, libcurl, json-c

---

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Core Components](#core-components)
3. [Command System](#command-system)
4. [Shell Implementation](#shell-implementation)
5. [AI Systems](#ai-systems)
6. [Package Manager](#package-manager)
7. [Process Execution](#process-execution)
8. [Grammar and Parsing](#grammar-and-parsing)
9. [Build System](#build-system)
10. [Technical Details](#technical-details)

---

## System Architecture

### High-Level Design

PicoBox follows a modular architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│                    Main Entry Point                     │
│                    (src/main.c)                         │
└─────────────┬───────────────────────────────────────────┘
              │
    ┌─────────┴─────────┐
    │                   │
┌───▼────────┐    ┌────▼──────────┐
│  Command   │    │  Interactive  │
│   Mode     │    │  Shell Mode   │
└───┬────────┘    └────┬──────────┘
    │                  │
    │           ┌──────┴──────┐
    │           │             │
    │      ┌───▼────┐   ┌────▼─────────┐
    │      │ Simple │   │ BNFC-Powered │
    │      │ Shell  │   │    Shell     │
    │      └────────┘   └──────────────┘
    │
┌───▼─────────────────────────────────────────────────────┐
│             Command Registry & Execution                │
│  (27+ Commands + Package Manager + AI Assistants)      │
└─────────────────────────────────────────────────────────┘
```

### Key Design Principles

1. **Single Binary Multiple Tools**: One executable (`picobox`) provides all commands through symlink dispatch
2. **Command Registry Pattern**: Commands self-register at startup with metadata
3. **Visitor Pattern**: AST traversal for shell command execution
4. **Fork/Exec Model**: Proper process isolation for command execution
5. **Grammar-Based Parsing**: BNFC-generated parser ensures correct syntax handling

---

## Core Components

### 1. Main Entry Point (`src/main.c`)

**Purpose:** Bootstrap the system and dispatch to appropriate mode.

**Key Functions:**
- `main(int argc, char **argv)` - Entry point
- `init_commands(void)` - Register all commands
- `find_legacy_command(const char *name)` - Legacy command lookup
- `print_commands_json(void)` - JSON export for AI integration
- `print_usage(void)` - Help system

**Execution Flow:**
```
1. Initialize command registry (register 27+ commands)
2. Check for special flags (--commands-json, --help)
3. Determine invocation mode:
   - Called as "picobox" → shell mode or command dispatcher
   - Called via symlink (e.g., "ls") → direct command execution
4. Parse arguments and execute
```

**Special Features:**
- Environment variable `PICOBOX_BNFC=1` selects advanced shell
- Supports both built-in commands and external execution
- Never uses `exit()` - all commands return status codes

### 2. Command Registry (`src/core/registry.c`)

**Purpose:** Central repository for command metadata and function pointers.

**Data Structure:**
```c
typedef struct cmd_spec {
    const char *name;           // Command name (e.g., "ls")
    const char *summary;        // One-line description
    const char *long_help;      // Detailed help text
    int (*run)(int argc, char **argv);      // Execution function
    void (*print_usage)(FILE *out);         // Help printer
} cmd_spec_t;
```

**Key Functions:**
- `register_command(const cmd_spec_t *spec)` - Add command to registry
- `find_command(const char *name)` - Lookup by name
- `for_each_command(callback, userdata)` - Iterate all commands

**Implementation Details:**
- Static array of 64 command slots (expandable)
- Linear search O(n) - fast enough for small n
- Thread-unsafe (single-threaded shell)
- Commands registered during `init_commands()` phase

### 3. Command Specification (`include/cmd_spec.h`)

**Purpose:** Define standard anatomy for all commands.

**Standard Command Structure:**
```c
// Section 1: Argtable structures (argument parsing)
static struct arg_lit *cmd_help;
static struct arg_str *cmd_args;
static struct arg_end *cmd_end;

// Section 2: Argtable builder
static void build_cmd_argtable(void) { ... }

// Section 3: Run function (main implementation)
int cmd_run(int argc, char **argv) { ... }

// Section 4: Print usage function
void cmd_print_usage(FILE *out) { ... }

// Section 5: Command specification
cmd_spec_t cmd_spec = { .name = "...", ... };

// Section 6: Registration function
void register_cmd_command(void) { ... }

// Section 7: Standalone main (optional)
#ifndef BUILTIN_ONLY
int main(int argc, char **argv) { ... }
#endif
```

**Benefits:**
- Consistent interface across all commands
- Commands can be built standalone or as built-ins
- Self-documenting with argtable3 integration
- Easy to add new commands

---

## Command System

### Available Commands (27+)

#### File Operations (10 commands)
- **cat** - Concatenate and display files
- **cp** - Copy files and directories
- **mv** - Move/rename files
- **rm** - Remove files and directories
- **ls** - List directory contents
- **ln** - Create symbolic/hard links
- **touch** - Create empty files or update timestamps
- **mkdir** - Create directories
- **chmod** - Change file permissions
- **stat** - Display file status

#### Text Processing (5 commands)
- **echo** - Print text to stdout
- **head** - Display first lines of file
- **tail** - Display last lines of file
- **wc** - Count words, lines, characters
- **grep** - Search for patterns in files

#### Path Utilities (4 commands)
- **pwd** - Print working directory
- **basename** - Extract filename from path
- **dirname** - Extract directory from path
- **find** - Search for files in directory hierarchy

#### System Information (3 commands)
- **env** - Display environment variables
- **df** - Display disk space usage
- **du** - Estimate file/directory space usage

#### Process Control (3 commands)
- **sleep** - Delay for specified time
- **true** - Return success (exit 0)
- **false** - Return failure (exit 1)

#### Special Commands (3 commands)
- **pkg** - Package manager (install/remove/list packages)
- **AI** - AI assistant (OpenAI-powered help)
- **@query** - Natural language command suggestions (RAG + LLM)

### Command Implementation Pattern

Example: `cmd_echo.c` (simplified)

```c
int echo_run(int argc, char **argv) {
    // 1. Build argtable definition
    build_echo_argtable();

    // 2. Parse arguments
    nerrors = arg_parse(argc, argv, echo_argtable);

    // 3. Handle --help
    if (echo_help->count > 0) {
        echo_print_usage(stdout);
        return EXIT_OK;
    }

    // 4. Handle parsing errors
    if (nerrors > 0) {
        arg_print_errors(stderr, echo_end, "echo");
        return EXIT_ERROR;
    }

    // 5. Execute command logic
    for (i = 0; i < echo_args->count; i++) {
        printf("%s%s", (i > 0 ? " " : ""), echo_args->sval[i]);
    }
    if (!echo_no_newline->count) printf("\n");

    // 6. Cleanup and return
    arg_freetable(echo_argtable, 4);
    return EXIT_OK;
}
```

---

## Shell Implementation

### Two Shell Modes

#### 1. Simple Shell (`src/shell.c`)
- Basic command parsing (manual string splitting)
- Limited features (no pipes, no redirections)
- Used as fallback or for testing

#### 2. BNFC-Powered Shell (`src/shell_bnfc.c`)
- **Grammar-based parsing** using BNFC (BNF Converter)
- **Full feature set:**
  - Simple commands: `ls -la`
  - Pipelines: `cat file | grep pattern | wc -l`
  - Redirections: `cmd < input.txt > output.txt >> append.txt`
  - Command sequences: `cmd1 ; cmd2 ; cmd3`
  - AI queries: `@show me all files` or `AI how do I list files`
- **Proper process isolation** using fork/exec
- **Visitor pattern** for AST traversal

### Shell Features

#### Built-in Commands
Commands that must run in the parent process (cannot fork):
- **cd [DIR]** - Change directory (modifies parent's working directory)
- **exit** - Exit shell (returns -1 signal)
- **help** - Display help information

#### External Commands
All other commands run in child processes via fork/exec.

#### Pipeline Execution (`src/pipe_helpers.c`)

**Algorithm:**
```
For pipeline: cmd1 | cmd2 | cmd3
1. For each command (i = 0 to 2):
   a. If not last command: create pipe
   b. Fork child process
   c. In child:
      - If not first: redirect stdin to previous pipe read end
      - If not last: redirect stdout to current pipe write end
      - Execute command with execvp()
   d. In parent:
      - Close previous pipe ends
      - Save current pipe for next iteration
2. Close all pipes in parent
3. Wait for all children
4. Return exit status of last command
```

**Example Flow:**
```
cmd1 | cmd2 | cmd3

Parent creates pipe0
├─ Child 1: stdout → pipe0[1], execvp("cmd1")
Parent creates pipe1
├─ Child 2: stdin ← pipe0[0], stdout → pipe1[1], execvp("cmd2")
Parent creates Child 3
├─ Child 3: stdin ← pipe1[0], execvp("cmd3")
Parent waits for all children
```

#### I/O Redirection (`src/redirect_helpers.c`)

**Supported Types:**
- `< file` - Input redirection (stdin from file)
- `> file` - Output redirection (stdout to file, truncate)
- `>> file` - Append redirection (stdout to file, append)

**Implementation:**
```c
int apply_redirections(struct redirection *redirs, int count) {
    for (each redirection) {
        int fd = open(filename, flags, mode);
        if (type == INPUT)  dup2(fd, STDIN_FILENO);
        if (type == OUTPUT) dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}
```

**Important:** Redirections are applied in child process before `execvp()`.

---

## AI Systems

PicoBox includes **two AI assistant systems** for helping users:

### 1. Legacy AI Command (`src/commands/cmd_ai.c`)

**Usage:** `AI <question>`

**Features:**
- Direct OpenAI API integration (gpt-3.5-turbo)
- Grammar-based invocation (part of shell grammar)
- Uses libcurl for HTTP requests
- Uses json-c for JSON parsing

**Flow:**
```
User types: AI how do I list all files
    ↓
1. Parser recognizes AICmd grammar rule
2. cmd_ai_run() combines words into query
3. make_openai_request() sends HTTP POST to OpenAI API
4. Response parsed and displayed to user
```

**Configuration:**
- Requires `AI_SHELL` environment variable with OpenAI API key
- Model: gpt-3.5-turbo (configurable)
- Temperature: 0.3 (focused responses)
- Max tokens: 150

**System Prompt:**
```
You are a helpful Unix shell assistant for PicoBox, a BNFC-powered shell.
- For "how do I" questions: Provide ONLY the command, no explanation
- For "what is" questions: Brief, friendly explanation
- No markdown, just plain text
```

### 2. RAG-Enhanced AI Helper (`mysh_llm.py`)

**Usage:** `@<natural language query>`

**Features:**
- **RAG (Retrieval-Augmented Generation)** - scores commands by relevance
- **LLM Integration** - uses OpenAI API for smart suggestions
- **Fallback Heuristics** - works without API key
- **Command Catalog** - reads available commands via `picobox --commands-json`

**Architecture:**
```
User types: @show me all files
    ↓
1. Shell detects @ prefix (before BNFC parsing)
2. handle_llm_query() calls Python script
3. Python script:
   a. Load command catalog (picobox --commands-json)
   b. Score commands by relevance (RAG)
   c. Build prompt with top 5 relevant commands
   d. Call OpenAI API (or use heuristic fallback)
   e. Return suggested command
4. Shell displays suggestion with confirmation prompt
5. If user approves (y/n):
   a. Parse suggestion using BNFC
   b. Execute via visitor pattern
```

**RAG Scoring Algorithm:**
```python
def score_command(query, cmd):
    score = 0
    for word in query.split():
        if word in cmd.name:        score += 3
        if word in cmd.summary:     score += 2
        if word in cmd.description: score += 1
    return score
```

**Environment Variables:**
- `OPENAI_API_KEY` or `AI_SHELL` - API key
- `MYSH_LLM_MODEL` - Model to use (default: gpt-4o-mini)
- `MYSH_LLM_DEBUG` - Enable debug output (set to 1)
- `MYSH_LLM_SCRIPT` - Path to Python script

**Example Session:**
```bash
$ @show me all files

💡 AI Suggested Command:
   ls -la

Run this command? (y/n): y

total 120
drwxr-xr-x  15 user  staff   480 Dec  5 10:30 .
drwxr-xr-x   8 user  staff   256 Dec  4 09:15 ..
-rw-r--r--   1 user  staff  1024 Dec  5 10:25 README.md
...
```

### Comparison: AI Command vs @ Query

| Feature | AI Command | @ Query |
|---------|-----------|---------|
| Invocation | `AI <question>` | `@<query>` |
| Grammar Integration | Yes (AICmd rule) | No (pre-parser) |
| Command Catalog | No | Yes (RAG) |
| Execution | Manual | Auto (with confirmation) |
| Fallback | None | Heuristics |
| Response Style | Conversational | Command-only |

---

## Package Manager

### Overview

`pkg` command provides package management for PicoBox, similar to apt/yum/brew.

**Installation Directory:** `~/.mysh/`
- `~/.mysh/packages/` - Installed packages
- `~/.mysh/bin/` - Executable symlinks
- `~/.mysh/pkgdb.json` - Package database

### Package Format

Packages are `.tar.gz` archives containing:
- `pkg.json` - Metadata file
- Binary files and resources

**pkg.json Example:**
```json
{
  "name": "hello",
  "version": "1.0.0",
  "description": "Hello world program",
  "binaries": ["hello"]
}
```

### Commands

#### Install Package
```bash
pkg install hello-1.0.0.tar.gz
```

**Process:**
1. Extract tar.gz to temp directory
2. Parse pkg.json metadata
3. Check if already installed
4. Copy files to `~/.mysh/packages/<name>-<version>/`
5. Create symlinks in `~/.mysh/bin/` for binaries
6. Update package database (`pkgdb.json`)

#### List Packages
```bash
pkg list
```
Output:
```
Installed packages:
NAME                 VERSION      DESCRIPTION
----                 -------      -----------
hello                1.0.0        Hello world program

Total: 1 package
```

#### Package Info
```bash
pkg info hello
```
Output:
```
Package: hello
Version: 1.0.0
Description: Hello world program
Installed: 2025-12-05
Location: /Users/user/.mysh/packages/hello-1.0.0

Files:
  hello
  pkg.json
```

#### Remove Package
```bash
pkg remove hello
```

**Process:**
1. Find package in database
2. Remove package directory
3. Remove symlinks from bin/
4. Update package database

### Implementation Details

**Key Design Decisions:**
1. **Simple JSON parsing** - String searches instead of external parser (for pkg.json only)
2. **Fork/exec for tar** - Uses system tar command for extraction
3. **goto cleanup pattern** - Ensures temp directories are always cleaned up
4. **Symlink management** - Binaries accessible via `~/.mysh/bin/` in PATH

**Database Format (`pkgdb.json`):**
```json
{
  "installed": [
    {
      "name": "hello",
      "version": "1.0.0",
      "description": "Hello world program",
      "date": "2025-12-05",
      "path": "/Users/user/.mysh/packages/hello-1.0.0"
    }
  ]
}
```

---

## Process Execution

### Execution Helpers (`src/exec_helpers.c`)

#### Built-in Check
```c
int is_builtin(const char *cmd) {
    return strcmp(cmd, "cd") == 0 ||
           strcmp(cmd, "exit") == 0 ||
           strcmp(cmd, "help") == 0;
}
```

#### External Command Execution
```c
int exec_command_external(char **argv) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(argv[0], argv);
        perror(argv[0]);
        _exit(127);  // Command not found
    }
    // Parent waits for child
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}
```

#### Command with Redirections
```c
int exec_command_with_redirects(char **argv,
                                struct redirection *redirs,
                                int redir_count) {
    pid_t pid = fork();
    if (pid == 0) {
        // Apply redirections in child
        apply_redirections(redirs, redir_count);
        // Execute command
        execvp(argv[0], argv);
        _exit(127);
    }
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}
```

### Process Model

**Fork/Exec Pattern:**
```
Parent Process (Shell)
    ├─ fork() → Child Process
    │           ├─ Setup redirections (dup2)
    │           ├─ Setup pipes (dup2)
    │           └─ execvp(command) → Replace with new program
    └─ waitpid() → Wait for child to complete
```

**Why Fork/Exec?**
- **Process isolation** - Command failures don't crash shell
- **Resource cleanup** - Child resources freed automatically
- **Standard Unix model** - Compatible with all Unix programs
- **Signal handling** - Proper Ctrl+C behavior

---

## Grammar and Parsing

### BNFC (BNF Converter)

PicoBox uses BNFC to generate a parser from a BNF grammar specification.

**Grammar File:** `bnfc_shell/Shell.cf`

**Key Grammar Rules:**
```bnf
-- Top-level input
Input ::= ListCommand ";" ;

-- Commands
Command ::= SimpleCommand           -- Single command
          | Pipeline                 -- Commands with pipes
          | "AI" ListWord ;          -- AI assistant

-- Pipelines
Pipeline ::= ListSimpleCommand "|" ;

-- Simple command with redirections
SimpleCommand ::= Word ListWord ListRedirection ;

-- Redirections
Redirection ::= "<" Word            -- Input
              | ">" Word            -- Output
              | ">>" Word ;         -- Append
```

**Generated Files:**
- `Lexer.c/.h` - Tokenizer (from Flex)
- `Parser.c/.h` - Parser (from Bison)
- `Absyn.c/.h` - Abstract Syntax Tree definitions
- `Skeleton.c/.h` - Visitor pattern traversal

### AST Visitor Pattern (`bnfc_shell/Skeleton.c`)

**Purpose:** Traverse and execute parsed AST.

**Execution Context:**
```c
typedef struct ExecContext {
    int should_exit;        // Exit signal from 'exit' command
    int has_error;          // Error flag
    int last_exit_status;   // Last command exit status
} ExecContext;
```

**Visitor Functions:**
```c
void visitInput(Input ast, ExecContext *ctx);
void visitCommand(Command ast, ExecContext *ctx);
void visitSimpleCommand(SimpleCommand ast, ExecContext *ctx);
void visitPipeline(Pipeline ast, ExecContext *ctx);
```

**Example Visitor Implementation:**
```c
void visitCommand(Command ast, ExecContext *ctx) {
    switch (ast->kind) {
        case is_SimpleCmd:
            visitSimpleCommand(ast->u.simpleCmd_.simplecommand_, ctx);
            break;
        case is_PipeCmd:
            visitPipeline(ast->u.pipeCmd_.pipeline_, ctx);
            break;
        case is_AICmd:
            execute_ai_command(ast->u.aICmd_.listword_, ctx);
            break;
    }
}
```

### Parsing Flow

```
User Input: ls -la | grep test
    ↓
1. Lexer tokenizes: [WORD "ls", WORD "-la", PIPE, WORD "grep", WORD "test"]
    ↓
2. Parser builds AST:
   Input
    └─ ListCommand
        └─ Command (PipeCmd)
            └─ Pipeline
                ├─ SimpleCommand("ls", ["-la"])
                └─ SimpleCommand("grep", ["test"])
    ↓
3. Visitor executes:
   visitInput()
    → visitCommand() [PipeCmd]
     → visitPipeline()
      → exec_pipeline([["ls", "-la"], ["grep", "test"]])
       → Fork 2 children, setup pipe, execute
```

---

## Build System

### Makefile Structure

**Key Targets:**
- `make all` - Build picobox binary (default)
- `make clean` - Remove build artifacts
- `make install` - Install to /usr/local/bin with symlinks
- `make test` - Run test suite
- `make standalone` - Build commands as separate binaries

**Compilation Flags:**
- `-Wall -Wextra -Werror` - Strict error checking
- `-std=c11` - C11 standard
- `-O2` - Optimization level 2
- `-g` - Debug symbols

**Dependencies:**
- argtable3 - Argument parsing
- libcurl - HTTP requests (for AI features)
- json-c - JSON parsing (for AI features)
- bison - Parser generator
- flex - Lexer generator

### Build Process

```
1. Generate BNFC parser files (if grammar changed)
   cd bnfc_shell && make
    → Shell.tab.c (Bison parser)
    → lex.yy.c (Flex lexer)
    → Absyn.c (AST definitions)
    → Skeleton.c (Visitor pattern)

2. Compile core infrastructure
   src/core/registry.c → build/core/registry.o

3. Compile commands (with -DBUILTIN_ONLY)
   src/commands/cmd_*.c → build/refactored_cmd_*.o

4. Compile main source files
   src/main.c, src/shell_bnfc.c, etc. → build/*.o

5. Compile BNFC-generated files (relaxed warnings)
   bnfc_shell/*.c → build/bnfc_*.o

6. Link all objects into final binary
   build/picobox
```

### Directory Structure

```
picobox/
├── src/
│   ├── main.c                  # Entry point
│   ├── shell_bnfc.c            # BNFC-powered shell
│   ├── shell.c                 # Simple shell
│   ├── pipe_helpers.c          # Pipeline execution
│   ├── redirect_helpers.c      # I/O redirection
│   ├── exec_helpers.c          # Process execution
│   ├── core/
│   │   └── registry.c          # Command registry
│   └── commands/
│       ├── cmd_echo.c          # Refactored commands
│       ├── cmd_cat.c
│       ├── ... (27+ commands)
│       └── cmd_pkg.c           # Package manager
├── include/
│   ├── picobox.h               # Main header
│   ├── cmd_spec.h              # Command specification
│   ├── exec_helpers.h
│   ├── pipe_helpers.h
│   └── redirect_helpers.h
├── bnfc_shell/
│   ├── Shell.cf                # Grammar specification
│   ├── Makefile
│   └── (generated files)
├── build/                      # Build artifacts
├── tests/                      # Test scripts
├── mysh_llm.py                 # AI helper script
├── Makefile
└── README.md
```

---

## Technical Details

### Memory Management

**Principles:**
1. **No memory leaks** - All allocations have corresponding frees
2. **RAII pattern** - Cleanup at function exit (or via goto cleanup)
3. **Argtable cleanup** - `arg_freetable()` in all command functions
4. **AST cleanup** - `free_Input()` after execution

**Example Pattern:**
```c
int command_function(int argc, char **argv) {
    char **argv_list = NULL;
    struct redirection *redirs = NULL;

    // Allocations
    argv_list = malloc(...);
    redirs = malloc(...);

    // Use allocated resources
    int result = do_work(argv_list, redirs);

    // Always cleanup (even on error)
cleanup:
    if (argv_list) free_argv(argv_list);
    if (redirs) free(redirs);
    return result;
}
```

### Error Handling

**Principles:**
1. **No exit() in commands** - Always return status codes
2. **perror() for system errors** - Standard error reporting
3. **fprintf(stderr, ...)** for user errors
4. **Status codes:** EXIT_OK (0), EXIT_ERROR (1), -1 (exit signal)

**Example:**
```c
int cmd_function(int argc, char **argv) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);              // System error
        return EXIT_ERROR;
    }

    if (invalid_argument) {
        fprintf(stderr, "Invalid argument\n");  // User error
        return EXIT_ERROR;
    }

    return EXIT_OK;  // Success
}
```

### Security Considerations

**Current Implementation:**
1. **No shell injection protection** - Assumes trusted input
2. **No sandboxing** - Commands have full system access
3. **API key in environment** - Cleartext storage

**Future Improvements:**
1. Input validation and sanitization
2. Secure storage for API keys
3. Command whitelist/blacklist
4. Resource limits (ulimit integration)

### Performance Characteristics

**Command Execution:**
- Fork/exec overhead: ~1-5ms per command
- Pipeline with 3 commands: ~5-15ms
- AI query (with network): ~500-2000ms

**Memory Usage:**
- Base shell: ~2MB
- Per command: ~1-5MB (copy-on-write)
- Parser: ~500KB

**Scalability:**
- Commands: Up to 64 registered
- Pipeline: Unlimited stages (memory permitting)
- Arguments: Up to 100 per command (argtable limit)

### Platform Support

**Tested Platforms:**
- macOS (Darwin 24.3.0)
- Linux (Ubuntu, Debian, Fedora)
- BSD systems (likely compatible)

**Dependencies:**
- POSIX API (fork, exec, pipe, dup2)
- C11 compiler (GCC, Clang)
- GNU Make or compatible
- Bison 3.x
- Flex 2.x

### Known Limitations

1. **No background jobs** - No & operator support
2. **No job control** - No fg/bg/jobs commands
3. **No command substitution** - No $(cmd) or `cmd`
4. **No environment variable expansion** - No $VAR
5. **No wildcards** - No glob expansion (*, ?, [...])
6. **No quotes** - Limited string handling
7. **No pipes in redirections** - Redirect breaks pipe chain

### Exit Codes

**Standard Codes:**
- `0` - Success (EXIT_OK)
- `1` - General error (EXIT_ERROR)
- `127` - Command not found
- `128+N` - Killed by signal N

**Special Codes:**
- `-1` - Exit signal (shell should terminate)

---

## API Reference

### Command Registration API

```c
// Register a command (called during init_commands)
void register_command(const cmd_spec_t *spec);

// Find a registered command
const cmd_spec_t *find_command(const char *name);

// Iterate over all commands
void for_each_command(
    void (*callback)(const cmd_spec_t *spec, void *data),
    void *userdata
);
```

### Execution API

```c
// Execute external command
int exec_command_external(char **argv);

// Execute with redirections
int exec_command_with_redirects(
    char **argv,
    struct redirection *redirs,
    int redir_count
);

// Execute pipeline
int exec_pipeline(char ***argv_list, int count);

// Check if command is builtin
int is_builtin(const char *cmd);
```

### Visitor Pattern API

```c
// Create execution context
ExecContext *exec_context_new(void);

// Free execution context
void exec_context_free(ExecContext *ctx);

// Visit AST nodes
void visitInput(Input ast, ExecContext *ctx);
void visitCommand(Command ast, ExecContext *ctx);
void visitSimpleCommand(SimpleCommand ast, ExecContext *ctx);
void visitPipeline(Pipeline ast, ExecContext *ctx);
```

---

## Version History

**v0.1.0** (Current)
- 27+ Unix commands implemented
- BNFC-powered shell with pipelines and redirections
- Dual AI assistant systems (RAG + LLM)
- Package manager (pkg install/remove/list)
- Visitor pattern for AST execution
- Fork/exec process model
- Command registry infrastructure

---

## Future Roadmap

### Phase 8: Background Jobs
- `&` operator for background execution
- `jobs` command to list background jobs
- `fg`/`bg` commands for job control
- Signal handling (SIGCHLD)

### Phase 9: Environment Variables
- `$VAR` expansion
- `export VAR=value`
- `unset VAR`

### Phase 10: Advanced Features
- Command substitution `$(cmd)` and `` `cmd` ``
- Glob expansion (`*.txt`, `file[1-3].c`)
- Quoted strings with escaping
- Command history and editing (readline)
- Tab completion

### Phase 11: Scripting
- Conditionals (`if`, `else`)
- Loops (`for`, `while`)
- Functions
- Script file execution

---

## References

### External Documentation
- [BNFC Official Documentation](https://bnfc.digitalgrammars.com/)
- [Argtable3 Documentation](https://www.argtable.org/)
- [OpenAI API Reference](https://platform.openai.com/docs)
- [POSIX.1-2017 Standard](https://pubs.opengroup.org/onlinepubs/9699919799/)

### Internal Documentation
- [README.md](README.md) - Quick start guide
- [GETTING_STARTED.md](GETTING_STARTED.md) - User guide
- Source code comments and docstrings

---

## Contributing

### Code Style
- C11 standard
- 4-space indentation
- Max 100 characters per line
- Function comments in multi-line style
- Return codes: EXIT_OK, EXIT_ERROR

### Adding a New Command

1. Create `src/commands/cmd_newcmd.c`
2. Follow 7-section anatomy (see cmd_echo.c)
3. Add `extern void register_newcmd_command()` to main.c
4. Call `register_newcmd_command()` in init_commands()
5. Add entry to dispatch table in main.c
6. Recompile: `make rebuild`

### Testing
- Add tests to `tests/test_newcmd.sh`
- Run `make test`
- Check for memory leaks: `make valgrind`

---

## License

MIT License (see LICENSE file)

---

## Contact

For bugs, feature requests, or questions:
- GitHub Issues: [project repository]
- Documentation: This file

---

**End of Documentation**
