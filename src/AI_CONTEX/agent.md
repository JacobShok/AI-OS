# AGENT.md: Programming Best Practices for AI Integration
## Test-Driven Development & Professional Standards

---

## 🎯 **CORE PHILOSOPHY**

> **"Test after EVERY change. Commit after EVERY successful test. Document as you go."**

This document provides **prescriptive guidance** on writing professional, maintainable code for the AI integration project. Follow these practices religiously to avoid bugs, facilitate debugging, and produce production-quality software.

---

## 📐 **FUNDAMENTAL PRINCIPLES**

### **1. The Testing Pyramid**

```
        /\
       /  \      Unit Tests (Fast, Many)
      /____\
     /      \    Integration Tests (Medium Speed, Some)
    /________\
   /          \  End-to-End Tests (Slow, Few)
  /__________\
```

**Application to This Project**:

- **Unit Tests**: Test each function independently
  - `print_commands_json()` → Valid JSON?
  - `score_command()` → Correct scores?
  - `handle_llm_query()` → Proper error handling?

- **Integration Tests**: Test component interactions
  - Shell calls Python script → Output captured?
  - Python calls shell --commands-json → JSON parsed?
  - Suggestion parsed by BNFC → AST correct?

- **End-to-End Tests**: Test complete workflows
  - User types `@list files` → Command executes?
  - Error recovery → Shell still usable?

### **2. The Red-Green-Refactor Cycle**

```
┌──────────────┐
│   RED        │  Write test that fails
│   (Failing)  │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   GREEN      │  Write minimum code to pass
│   (Passing)  │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   REFACTOR   │  Clean up code
│   (Better)   │
└──────┬───────┘
       │
       └────────► Repeat
```

**Example**:

```bash
# RED: Test fails
./picobox --commands-json
# Expected: JSON
# Actual: Error

# GREEN: Implement minimum
# Add print_commands_json() with hardcoded output
./picobox --commands-json
# Now returns: {"commands":[]}

# REFACTOR: Make it real
# Actually iterate through commands array
./picobox --commands-json
# Now returns: {"commands":[{"name":"echo",...},...]}

# COMMIT
git add main.c
git commit -m "Add --commands-json flag"
```

### **3. Single Responsibility Principle**

**Each function should do ONE thing and do it well.**

❌ **Bad Example**:
```c
void handle_ai_query(const char *query) {
    // 1. Build command
    // 2. Call Python
    // 3. Read output
    // 4. Display to user
    // 5. Get confirmation
    // 6. Parse suggestion
    // 7. Execute command
    // TOO MANY RESPONSIBILITIES!
}
```

✅ **Good Example**:
```c
void handle_llm_query(const char *query) {
    char *suggestion = get_ai_suggestion(query);
    if (!suggestion) return;
    
    if (confirm_with_user(suggestion)) {
        execute_suggestion(suggestion);
    }
    
    free(suggestion);
}

char *get_ai_suggestion(const char *query) {
    // Only responsible for getting suggestion
}

int confirm_with_user(const char *cmd) {
    // Only responsible for confirmation
}

void execute_suggestion(const char *cmd) {
    // Only responsible for execution
}
```

---

## 🔍 **CODE QUALITY STANDARDS**

### **Error Handling**

**RULE: Check EVERY system call, library function, and allocation.**

❌ **Lazy Code** (Don't do this):
```c
FILE *fp = fopen("file.txt", "r");
fgets(buffer, sizeof(buffer), fp);  // CRASH if fp is NULL!
```

✅ **Professional Code**:
```c
FILE *fp = fopen("file.txt", "r");
if (!fp) {
    perror("fopen");
    return ERROR_FILE_NOT_FOUND;
}

if (fgets(buffer, sizeof(buffer), fp) == NULL) {
    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return ERROR_READ_FAILED;
    }
    // EOF is okay
}

fclose(fp);
```

**Checklist for Error Handling**:

- [ ] Check return value of `malloc()`, `calloc()`, `realloc()`
- [ ] Check return value of `fopen()`, `popen()`
- [ ] Check return value of `fgets()`, `fread()`
- [ ] Check return value of `fork()`, `exec()`
- [ ] Check return value of `pipe()`, `dup2()`
- [ ] Check return value of `psInput()` (BNFC parser)
- [ ] Check return value of `exec_context_new()`
- [ ] Use `perror()` or `fprintf(stderr, ...)` for errors
- [ ] Clean up resources before returning on error

**Testing Error Handling**:

```bash
# Test malloc failure (hard to simulate, but check code)
# Test file not found
./picobox --commands-json > /dev/null 2>&1 || echo "Handle this!"

# Test popen failure
rm mysh_llm.py  # Temporarily remove script
./picobox
$ @test query
# Should print helpful error, not crash

# Test parse failure
./picobox
$ @test
# Python returns "xyz123nonsense"
# (y to execute)
# Should print "Parse error", not crash

# Restore script
git restore mysh_llm.py
```

### **Memory Management**

**RULE: Every `malloc` needs a `free`. Every `open` needs a `close`.**

✅ **RAII Pattern** (Resource Acquisition Is Initialization):

```c
int my_function(void) {
    char *buffer = NULL;
    FILE *fp = NULL;
    int result = ERROR;
    
    // Allocate resources
    buffer = malloc(1024);
    if (!buffer) goto cleanup;
    
    fp = fopen("file.txt", "r");
    if (!fp) goto cleanup;
    
    // Do work
    // ...
    
    result = SUCCESS;
    
cleanup:
    // Free resources (safe even if NULL)
    free(buffer);
    if (fp) fclose(fp);
    return result;
}
```

**Memory Leak Detection**:

```bash
# Compile with debug symbols
make CFLAGS="-g -O0"

# Run with valgrind
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./picobox << 'EOF'
@list files
n
ls
echo test
exit
EOF

# Expected output:
# LEAK SUMMARY:
#    definitely lost: 0 bytes in 0 blocks
#    indirectly lost: 0 bytes in 0 blocks
#    possibly lost: 0 bytes in 0 blocks
```

**Common Memory Leaks to Avoid**:

```c
// LEAK 1: Not freeing argv
char **argv = words_to_argv(...);
// ... use argv ...
free_argv(argv);  // DON'T FORGET!

// LEAK 2: Not freeing AST
Input ast = psInput(line);
// ... use ast ...
free_Input(ast);  // DON'T FORGET!

// LEAK 3: Not closing pipe
FILE *fp = popen(cmd, "r");
// ... use fp ...
pclose(fp);  // DON'T FORGET!

// LEAK 4: Not freeing context
ExecContext *ctx = exec_context_new();
// ... use ctx ...
exec_context_free(ctx);  // DON'T FORGET!
```

### **Input Validation**

**RULE: Never trust external input. Validate everything.**

```c
static void handle_llm_query(const char *query) {
    /* ============ VALIDATION BLOCK ============ */
    
    // Check 1: Null pointer
    if (!query) {
        fprintf(stderr, "Error: NULL query\n");
        return;
    }
    
    // Check 2: Empty string
    if (query[0] == '\0') {
        fprintf(stderr, "Error: Empty query\n");
        return;
    }
    
    // Check 3: Length limit
    if (strlen(query) > MAX_QUERY_LENGTH) {
        fprintf(stderr, "Error: Query too long (max %d chars)\n", 
                MAX_QUERY_LENGTH);
        return;
    }
    
    // Check 4: No dangerous characters (basic)
    if (strchr(query, '\n') || strchr(query, '\r')) {
        fprintf(stderr, "Error: Query contains newline\n");
        return;
    }
    
    /* ========================================== */
    
    // Now safe to process query
    // ...
}
```

**Testing Input Validation**:

```bash
# Test empty query
./picobox
$ @
Error: Empty query
$ 

# Test very long query
./picobox
$ @$(python3 -c "print('a'*10000)")
Error: Query too long
$ 

# Test query with newline
./picobox
$ @list$(echo -e '\n')files
Error: Query contains newline
$ 
```

---

## 🧪 **TESTING METHODOLOGY**

### **Test Levels**

For each change, run tests in this order:

#### **Level 1: Compilation Test**

```bash
# ALWAYS test compilation first
make clean
make 2>&1 | tee compile.log

# Check for warnings
grep -i warning compile.log
# Should be empty or only acceptable warnings

# Check exit code
echo $?
# Should be 0
```

**DO NOT PROCEED if compilation fails or has warnings.**

#### **Level 2: Unit Test**

Test the specific function you just wrote:

```bash
# For print_commands_json()
./picobox --commands-json | python3 -m json.tool > /dev/null
echo $?  # Should be 0

# For mysh_llm.py standalone
python3 mysh_llm.py "list files"
# Should print one line

# For handle_llm_query() (manual)
./picobox
$ @list files
# Should show suggestion
```

**DO NOT PROCEED if unit test fails.**

#### **Level 3: Integration Test**

Test interactions between components:

```bash
# Test: Shell calls Python
$ @list files
n
# Should see suggestion

# Test: Python calls shell
python3 mysh_llm.py "test"
# Should see command

# Test: Suggestion parsed by BNFC
$ @list files
y
# Should execute successfully
```

**DO NOT PROCEED if integration fails.**

#### **Level 4: Regression Test**

Ensure nothing broke:

```bash
# Test normal commands
$ ls
$ cat file.txt
$ echo test | grep test
$ exit

# Test legacy AI
$ AI how do I list files
$ exit

# Test pipelines
$ cat file | wc -l
$ exit

# Test redirections
$ echo test > file
$ cat < file
$ exit
```

**DO NOT PROCEED if any regression test fails.**

#### **Level 5: System Test**

Test complete workflows:

```bash
# Workflow 1: Happy path
$ @show all files
y
# Files listed

# Workflow 2: Cancel
$ @find C files
n
# Cancelled

# Workflow 3: Error recovery
$ @invalid query
# (suggestion might be weird)
n
$ ls  # Shell still works
```

### **Test Automation**

Create test script that runs after every change:

**File**: `quick_test.sh`

```bash
#!/bin/bash
# Quick smoke tests - run after every change

set -e
trap 'echo "❌ Test failed at line $LINENO"' ERR

echo "=== Quick Test Suite ==="

# Test 1: Compilation
echo "[1/5] Compilation..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1
echo "  ✓ Compiled"

# Test 2: JSON output
echo "[2/5] JSON output..."
./picobox --commands-json | python3 -m json.tool > /dev/null
echo "  ✓ JSON valid"

# Test 3: Python script
echo "[3/5] Python script..."
export MYSH_PATH=./picobox
export MYSH_CATALOG_CMD='./picobox --commands-json'
unset OPENAI_API_KEY
OUTPUT=$(python3 mysh_llm.py "list files")
[ -n "$OUTPUT" ] || exit 1
echo "  ✓ Script works"

# Test 4: Shell integration
echo "[4/5] Shell @ detection..."
OUTPUT=$(echo -e "@list files\nn\nexit" | ./picobox 2>&1 | grep "Suggested")
[ -n "$OUTPUT" ] || exit 1
echo "  ✓ @ detection works"

# Test 5: Normal commands
echo "[5/5] Regression..."
OUTPUT=$(echo -e "echo test\nexit" | ./picobox 2>&1 | grep "test")
[ -n "$OUTPUT" ] || exit 1
echo "  ✓ Normal commands work"

echo ""
echo "✅ All quick tests passed"
```

**Usage**:

```bash
chmod +x quick_test.sh

# After EVERY change
nano main.c
make
./quick_test.sh

# If all pass
git add main.c
git commit -m "Add feature X"
```

---

## 📝 **DOCUMENTATION STANDARDS**

### **Function Documentation Template**

```c
/*
 * <Function Name> - <One-line description>
 * 
 * <Detailed description explaining what the function does,
 * why it exists, and any important implementation details.>
 *
 * Parameters:
 *   param1 - Description of first parameter
 *   param2 - Description of second parameter
 * 
 * Returns:
 *   Description of return value
 *   - 0 on success
 *   - -1 on error (check errno)
 * 
 * Side Effects:
 *   - Modifies global variable X
 *   - Allocates memory that caller must free
 *   - Calls external program Y
 * 
 * Notes:
 *   - This function is NOT thread-safe
 *   - Query must not contain newlines
 *   - Caller is responsible for freeing returned string
 * 
 * Example:
 *   char *result = function_name("input");
 *   if (result) {
 *       printf("%s\n", result);
 *       free(result);
 *   }
 */
```

**Example Application**:

```c
/*
 * get_ai_suggestion - Get command suggestion from AI helper
 * 
 * Calls the Python mysh_llm.py script with the given query
 * and returns the suggested command as a newly allocated string.
 * 
 * This function:
 * 1. Builds command to call Python script
 * 2. Executes via popen()
 * 3. Reads one line of output (the suggestion)
 * 4. Returns the suggestion
 * 
 * Parameters:
 *   query - Natural language query (must not contain newlines)
 * 
 * Returns:
 *   - Newly allocated string with suggestion (caller must free)
 *   - NULL on error (script not found, no output, etc.)
 * 
 * Side Effects:
 *   - Forks a child process to run Python script
 *   - Prints error messages to stderr on failure
 * 
 * Notes:
 *   - Query is NOT escaped; avoid special shell characters
 *   - Script path from MYSH_LLM_SCRIPT env var or default
 *   - Timeout is handled by popen (system-dependent)
 * 
 * Example:
 *   char *suggestion = get_ai_suggestion("list files");
 *   if (suggestion) {
 *       printf("Suggested: %s\n", suggestion);
 *       free(suggestion);
 *   } else {
 *       fprintf(stderr, "Failed to get suggestion\n");
 *   }
 */
static char *get_ai_suggestion(const char *query)
{
    // Implementation...
}
```

### **Inline Comments**

**Explain WHY, not WHAT:**

❌ **Bad Comments**:
```c
// Check if line starts with @
if (line[0] == '@') {
    // Call handler
    handle_llm_query(line + 1);
    // Continue to next iteration
    continue;
}
```

✅ **Good Comments**:
```c
/* Check for AI query BEFORE parsing.
 * 
 * We handle @ queries before calling psInput() because:
 * 1. The @ character is not in our grammar
 * 2. This keeps both AI systems independent
 * 3. Parsing would fail on @ otherwise
 */
if (line[0] == '@') {
    handle_llm_query(line + 1);  // Skip @ character
    continue;  // Don't parse with BNFC
}
```

### **Commit Message Standards**

Follow the **Conventional Commits** format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Formatting changes
- `refactor`: Code restructuring
- `test`: Adding tests
- `chore`: Maintenance tasks

**Examples**:

```bash
# Good commit message
git commit -m "feat(ai): add @prefix detection in REPL loop

- Check for @ before calling psInput()
- Call handle_llm_query() with query text
- Continue to next iteration (skip parser)

This enables the new AI assistant feature while keeping
the grammar unchanged. Both AI systems now coexist.

Tested: @ detection, normal commands, legacy AI command
"

# Good commit message
git commit -m "fix(json): escape quotes in command descriptions

Command descriptions may contain quotes, which breaks JSON.
Now properly escape \" and \\ characters before printing.

Fixes: picobox --commands-json | python3 -m json.tool
"

# Good commit message
git commit -m "test(ai): add automated test suite

Create test_ai_integration.sh with 5 core tests:
1. JSON validation
2. Command count check
3. Python script standalone
4. Shell @ detection
5. Regression testing

All tests passing.
"
```

---

## 🔧 **DEBUGGING BEST PRACTICES**

### **Debugging Workflow**

When something doesn't work:

1. **Reproduce**: Can you make it fail consistently?
2. **Isolate**: Which component is failing?
3. **Simplify**: Remove complexity until it works
4. **Instrument**: Add logging to understand state
5. **Fix**: Make minimal change
6. **Verify**: Test that fix works
7. **Prevent**: Add test to catch regression

### **Debugging Tools**

#### **GDB (GNU Debugger)**

```bash
# Compile with debug symbols
make CFLAGS="-g -O0"

# Start GDB
gdb ./picobox

# Common commands
(gdb) break handle_llm_query    # Set breakpoint
(gdb) run                        # Start program
$ @test                          # Trigger breakpoint
(gdb) print query                # Inspect variable
(gdb) next                       # Execute next line
(gdb) step                       # Step into function
(gdb) continue                   # Continue execution
(gdb) backtrace                  # Show call stack
(gdb) quit                       # Exit
```

#### **Printf Debugging**

```c
// Add debug macros
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* nothing */
#endif

// Use in code
void handle_llm_query(const char *query) {
    DEBUG_PRINT("Query: %s", query);
    
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "python3 mysh_llm.py \"%s\"", query);
    DEBUG_PRINT("Command: %s", cmd);
    
    FILE *fp = popen(cmd, "r");
    DEBUG_PRINT("popen returned: %p", (void *)fp);
    
    // ... rest of function
}

// Compile with debug
make CFLAGS="-DDEBUG -g"
```

#### **Valgrind**

```bash
# Check memory leaks
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind.log \
         ./picobox

# Check output
less valgrind.log

# Look for:
# - "definitely lost": Real leaks (fix immediately)
# - "indirectly lost": Usually from "definitely lost"
# - "possibly lost": Might be false positive
# - "still reachable": Usually okay (global allocations)
```

#### **Strace (System Call Trace)**

```bash
# Trace system calls
strace -o trace.log ./picobox

# Look at trace
less trace.log

# Common syscalls to check:
# - open/close: File operations
# - read/write: I/O operations
# - fork/exec: Process creation
# - pipe: Pipe creation
# - dup2: File descriptor duplication
```

### **Common Bug Patterns**

#### **Bug: Segmentation Fault**

**Causes**:
1. NULL pointer dereference
2. Use after free
3. Buffer overflow
4. Stack overflow (deep recursion)

**Debugging**:
```bash
# Get core dump
ulimit -c unlimited
./picobox
# (crash)

# Analyze core dump
gdb ./picobox core
(gdb) backtrace
(gdb) frame 0
(gdb) print variable_name
```

#### **Bug: Memory Leak**

**Causes**:
1. Forgetting to free
2. Early return without cleanup
3. Lost pointer (can't free)

**Debugging**:
```bash
valgrind --leak-check=full ./picobox
# Look for "definitely lost"
# Note line numbers
# Add free() calls
```

#### **Bug: Undefined Behavior**

**Causes**:
1. Uninitialized variables
2. Out of bounds access
3. Integer overflow
4. Type punning

**Prevention**:
```bash
# Compile with all warnings
make CFLAGS="-Wall -Wextra -Werror -std=c11"

# Use static analysis
gcc -fanalyzer main.c

# Use sanitizers
make CFLAGS="-fsanitize=address -fsanitize=undefined"
./picobox
```

---

## 🚀 **DEVELOPMENT WORKFLOW**

### **Daily Workflow**

```bash
# 1. Start fresh
git status  # Clean working directory
git pull    # Get latest changes

# 2. Create branch for feature
git checkout -b feature/ai-integration

# 3. Make small change
nano main.c
# (Add print_commands_json() function)

# 4. Compile
make clean
make

# 5. Test immediately
./quick_test.sh

# 6. If test passes, commit
git add main.c
git commit -m "feat(json): add print_commands_json"

# 7. Continue with next change
nano main.c
# (Add --commands-json flag handler)
make
./quick_test.sh
git add main.c
git commit -m "feat(json): add --commands-json flag handler"

# 8. Run full tests periodically
./test_ai_integration.sh

# 9. When feature complete, merge
git checkout main
git merge feature/ai-integration
git branch -d feature/ai-integration
```

### **Incremental Development**

**Build features in layers:**

```
Layer 1: Basic JSON output (hardcoded)
  Test: picobox --commands-json
  Commit: "feat(json): add basic JSON output"

Layer 2: Real command iteration
  Test: Count commands in JSON
  Commit: "feat(json): iterate through command registry"

Layer 3: Add detailed info
  Test: Check fields in JSON
  Commit: "feat(json): add summary and description"

Layer 4: Handle edge cases
  Test: Commands with NULL descriptions
  Commit: "fix(json): handle NULL descriptions"
```

Each layer is:
- Small enough to debug easily
- Large enough to be meaningful
- Independently testable
- Immediately committable

---

## ✅ **CODE REVIEW CHECKLIST**

Before committing ANY code, verify:

### **Correctness**

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Logic is correct
- [ ] Edge cases handled
- [ ] Error cases handled

### **Safety**

- [ ] No buffer overflows
- [ ] No NULL pointer dereferences
- [ ] No memory leaks
- [ ] No use-after-free
- [ ] No uninitialized variables
- [ ] Proper input validation

### **Style**

- [ ] Consistent indentation (4 spaces or tabs)
- [ ] Function names are descriptive
- [ ] Variable names are clear
- [ ] Magic numbers replaced with constants
- [ ] No code duplication

### **Documentation**

- [ ] Functions have header comments
- [ ] Complex logic has inline comments
- [ ] Commit message is descriptive
- [ ] README updated if needed
- [ ] CHANGELOG updated if needed

### **Testing**

- [ ] Unit tests written
- [ ] Integration tests pass
- [ ] Regression tests pass
- [ ] Manual testing done
- [ ] Test script updated

---

## 🎓 **LEARNING FROM MISTAKES**

### **Post-Mortem Template**

When a bug is found in production:

```markdown
# Bug Post-Mortem: [Bug Title]

## What Happened
[Describe the bug and its impact]

## Root Cause
[What was the actual cause]

## Timeline
- [Time]: Bug introduced (which commit)
- [Time]: Bug discovered (how)
- [Time]: Bug fixed (which commit)

## Why Wasn't It Caught?
[Which tests should have caught it but didn't]

## What We Learned
[Lessons learned]

## Action Items
1. [Add test X to prevent recurrence]
2. [Update documentation Y]
3. [Refactor code Z]

## Related Issues
[Links to related bugs or discussions]
```

### **Common Mistakes to Avoid**

1. **Not checking return values**
   - Always check malloc, fopen, popen, etc.

2. **Assuming input is valid**
   - Always validate before use

3. **Forgetting to free memory**
   - Use valgrind regularly

4. **Not testing edge cases**
   - Empty input, very long input, special characters

5. **Committing broken code**
   - Always test before committing

6. **Not documenting why**
   - Comments explain why, not what

7. **Making too many changes at once**
   - Small commits are easier to debug

8. **Not running regression tests**
   - Always verify nothing broke

---

## 🏆 **QUALITY METRICS**

Track these metrics to ensure code quality:

### **Compilation Metrics**

```bash
# Warning count (should be 0)
make 2>&1 | grep -c warning

# Error count (should be 0)
make 2>&1 | grep -c error
```

### **Test Metrics**

```bash
# Test pass rate (should be 100%)
./test_ai_integration.sh
# All tests passed: 5/5 (100%)

# Code coverage (aim for >80%)
# (Requires instrumentation)
```

### **Code Quality Metrics**

```bash
# Lines of code
wc -l *.c

# Comment ratio (aim for >20%)
# (comments / total lines)

# Function length (aim for <50 lines per function)
# (Use static analysis tool)

# Cyclomatic complexity (aim for <10 per function)
# (Use static analysis tool)
```

### **Memory Metrics**

```bash
# Memory leaks (should be 0)
valgrind --leak-check=full ./picobox 2>&1 | \
    grep "definitely lost:" | \
    grep -o "[0-9,]* bytes"

# Heap usage (monitor trends)
valgrind --tool=massif ./picobox
ms_print massif.out.* | less
```

---

## 📚 **RESOURCES & REFERENCES**

### **C Programming**

- **Book**: "The C Programming Language" (K&R)
- **Style Guide**: Linux Kernel Coding Style
- **Best Practices**: CERT C Coding Standard

### **Testing**

- **Book**: "Test Driven Development" (Kent Beck)
- **Tool**: Valgrind User Manual
- **Practice**: Google Test C++ (for inspiration)

### **Git**

- **Guide**: "Pro Git" book (free online)
- **Standard**: Conventional Commits spec
- **Practice**: GitHub flow

### **Debugging**

- **Book**: "The Art of Debugging with GDB"
- **Guide**: GDB Tutorial
- **Tool**: rr (Record and Replay)

---

## 🎯 **SUCCESS CRITERIA**

You've achieved professional-quality code when:

✅ **Code compiles with `-Wall -Wextra -Werror`**
✅ **Valgrind reports zero leaks**
✅ **All tests pass automatically**
✅ **Code review checklist is satisfied**
✅ **Another developer can understand your code**
✅ **Documentation explains why decisions were made**
✅ **Git history tells a clear story**
✅ **You're proud to show this code to your professor**

---

## 💪 **COMMITMENT**

As a professional programmer, I commit to:

1. **Test after EVERY change** (even "small" ones)
2. **Commit only working code** (tests must pass)
3. **Document as I go** (not "later")
4. **Review my own code** (before committing)
5. **Fix warnings immediately** (not "eventually")
6. **Learn from mistakes** (write post-mortems)
7. **Help future maintainers** (including future me)

---

**Remember**: Professional code is code that works, is maintainable, and makes the next person's job easier. Take pride in your craft. Test thoroughly. Document clearly. Commit responsibly.

**"Any fool can write code that a computer can understand. Good programmers write code that humans can understand."** - Martin Fowler