# AI Agent Companion Guide for BNFC Shell Development
## Strategic AI-Assisted Development with Continuous Verification

**Version:** 1.0  
**Purpose:** Use AI effectively to accelerate BNFC shell development while maintaining deep understanding and correctness  
**Philosophy:** AI as a tool, not a crutch. Verify everything. Understand everything.

---

## 📋 Table of Contents

1. [How to Use This Guide](#how-to-use-this-guide)
2. [AI Interaction Principles](#ai-interaction-principles)
3. [Phase-by-Phase Prompting Strategies](#phase-by-phase-prompting-strategies)
4. [Verification Workflows](#verification-workflows)
5. [Common Pitfalls and Solutions](#common-pitfalls-and-solutions)
6. [Prompt Templates](#prompt-templates)
7. [Documentation Requirements](#documentation-requirements)

---

## How to Use This Guide

### The AI-Assisted Development Loop

```
1. PLAN
   ↓
2. PROMPT AI (specific, constrained)
   ↓
3. REVIEW AI OUTPUT (critically)
   ↓
4. IMPLEMENT (understand before typing)
   ↓
5. TEST (verify it works)
   ↓
6. DOCUMENT (log what you did)
   ↓
7. NEXT STEP
```

### Critical Rules

**✅ DO:**
- Ask AI to explain concepts before generating code
- Request minimal changes (diffs, not full rewrites)
- Verify EVERY suggestion before implementing
- Keep a prompt log of what works
- Test immediately after implementing suggestions

**❌ DON'T:**
- Blindly copy-paste AI-generated code
- Ask for "complete implementation" in one go
- Skip understanding the generated code
- Execute commands without reviewing them
- Move to next phase without passing tests

---

## AI Interaction Principles

### 1. Clarify Before Coding

**Pattern:**
```
YOU: "I need to implement X"
AI: [generates 200 lines of code]
❌ WRONG - you don't understand it yet
```

**Better:**
```
YOU: "Explain how X works in the context of Y"
AI: [explains concept]
YOU: "Now show me the minimal code change to add X"
AI: [shows small diff]
✅ RIGHT - you understand before implementing
```

### 2. Request Minimal Changes

**BAD prompt:**
```
"Implement pipe support for my shell"
```
Result: 500 lines of code you don't understand

**GOOD prompt:**
```
"Show me the BNFC grammar rule to add a pipe operator between two commands. 
Just the grammar rule, not full implementation."
```
Result: 2-3 lines you can verify

### 3. Verify Step-by-Step

**After each AI suggestion:**

```bash
1. Read the code carefully
2. Identify what it does
3. Ask: "Why this approach?"
4. Implement it yourself
5. Test it works
6. Commit if successful
```

### 4. Build Understanding

**When AI gives you code:**

```
DON'T: Copy and hope it works
DO:
  1. Ask AI to explain each part
  2. Modify it slightly to test understanding
  3. Implement similar code yourself
  4. Use AI's version as reference, not source
```

---

## Phase-by-Phase Prompting Strategies

### PHASE 1: Prerequisites & Setup

**Goal:** Install BNFC and verify toolchain

#### Prompt 1.1: Installation Guidance
```
Given that I'm on Ubuntu 22.04, what's the recommended way to install BNFC?
I need:
- BNFC itself
- Flex and Bison
- Any other dependencies

Provide commands I can verify before running.
```

**Expected AI response:**
- Package manager commands
- Verification commands
- Alternative installation methods

**Your action:**
```bash
# Review commands
# Understand what each does
# Run one at a time
# Verify each step
```

#### Prompt 1.2: Directory Structure
```
I need to organize a C project that will use BNFC for parsing.
The project currently has:
- main.c
- shell.c
- cmd_*.c files
- Makefile

What directory structure do you recommend for:
1. BNFC grammar files
2. Generated parser files
3. Test files
4. Integration with existing code

Explain the rationale for each choice.
```

**Verification:**
- [ ] Understand why each directory exists
- [ ] Can explain structure to someone else
- [ ] Create directories manually
- [ ] Verify structure makes sense

#### Prompt 1.3: Makefile Integration Planning
```
Before modifying my Makefile, explain:

1. How BNFC-generated files (.c, .h) should be compiled
2. What order files must be built in
3. How to make BNFC generation a build step
4. How to avoid recompiling when grammar doesn't change

Don't write the Makefile yet, just explain the strategy.
```

**Verification:**
- [ ] Draw the build dependency graph
- [ ] Understand each build step
- [ ] Can explain to someone else

---

### PHASE 2: BNFC Calculator Learning

**Goal:** Deeply understand BNFC workflow

#### Prompt 2.1: Grammar Explanation
```
I have this BNFC grammar for a calculator:

[paste Calc.cf]

Explain line-by-line:
1. What each rule means
2. How precedence is enforced
3. What AST nodes will be generated
4. How left/right recursion affects parsing

Use simple language and examples.
```

**Verification:**
- [ ] Can draw AST for "1 + 2 * 3"
- [ ] Can predict AST structure for any expression
- [ ] Understand why Term/Factor split matters

#### Prompt 2.2: Generated File Understanding
```
After running BNFC on Calc.cf, I have these files:
- Absyn.h
- Parser.h
- Skeleton.c

For each file, explain:
1. What it contains
2. What I need to modify (if anything)
3. What I must NOT modify
4. How files depend on each other

Don't show me the full contents, just explain the PURPOSE.
```

**Verification:**
- [ ] Open each file and verify AI's explanation
- [ ] Find the structures AI described
- [ ] Understand dependencies

#### Prompt 2.3: Visitor Pattern Deep Dive
```
I see this pattern in Skeleton.c:

void visitExp(Exp p) {
  switch(p->kind) {
    case is_EAdd:
      visitExp(p->u.eAdd_.exp_);
      visitTerm(p->u.eAdd_.term_);
      break;
    ...
  }
}

Explain:
1. Why use switch on p->kind?
2. What is p->u.eAdd_?
3. Why call visitExp and visitTerm recursively?
4. Where would I add my own code?
5. Walk through execution for "1 + 2"

Use a step-by-step trace with stack states.
```

**Verification:**
- [ ] Manually trace "1 + 2"
- [ ] Trace "1 + 2 * 3"
- [ ] Can predict execution order
- [ ] Understand recursion pattern

#### Prompt 2.4: Implementation Strategy
```
I want to make the calculator actually evaluate expressions.
DON'T write the code yet.

Instead, explain the strategy:
1. What data structure should I use (stack, etc)?
2. What should visitInteger do?
3. What should visitExp do for EAdd?
4. How does the result propagate?
5. Where do I print the final result?

Provide pseudocode, not C code.
```

**Verification:**
- [ ] Write pseudocode yourself
- [ ] Compare with AI's explanation
- [ ] Understand data flow completely

#### Prompt 2.5: Minimal Implementation
```
Based on the strategy we discussed, show me ONLY the code for:
1. Global stack declaration
2. visitInteger implementation

Nothing else. I want to implement visitExp myself.
```

**Verification:**
- [ ] Understand the 2-3 lines of code
- [ ] Implement visitExp yourself without AI
- [ ] Test with simple input
- [ ] Only ask AI to review if stuck

---

### PHASE 3: Simple Grammar Integration

**Goal:** Create and integrate basic shell grammar

#### Prompt 3.1: Grammar Design Discussion
```
I need a minimal shell grammar that supports:
- Command name + arguments
- Multiple commands separated by semicolon

BEFORE showing me the grammar, explain:
1. What nonterminals I'll need
2. What terminals I'll need
3. How to represent a list of words
4. How to separate commands

Use BNF notation concepts, don't write BNFC yet.
```

**Verification:**
- [ ] Draw grammar on paper
- [ ] Verify it accepts valid inputs
- [ ] Verify it rejects invalid inputs

#### Prompt 3.2: Token Definition
```
In BNFC, I need a token for command names and filenames.
It should match:
- Letters, digits
- Underscore, dot, slash, hyphen

Explain:
1. What a 'token' means in BNFC
2. How it differs from a nonterminal
3. The regex syntax BNFC uses
4. Why this specific pattern

Then show me just the token definition.
```

**Verification:**
- [ ] Understand regex pattern
- [ ] Test pattern against examples
- [ ] Verify edge cases

#### Prompt 3.3: Minimal Grammar Draft
```
Using the concepts we discussed, show me a minimal Shell.cf that:
- Has Input as entry point
- Supports list of commands
- Each command is Word + list of Words
- Commands separated by semicolon

Keep it under 15 lines. Add comments explaining each rule.
```

**Verification:**
- [ ] Read grammar line by line
- [ ] Ensure you understand every rule
- [ ] Can draw AST for sample input
- [ ] Type it yourself, don't copy-paste

#### Prompt 3.4: Integration Planning
```
I have an existing shell.c that parses manually.
I want to integrate BNFC parsing.

Explain the integration strategy:
1. Where to call the parser (psInput or pInput)?
2. How to convert Input* to something usable?
3. What happens to existing parse_line()?
4. How to preserve built-ins (cd, exit)?
5. How to toggle between old/new shell?

No code yet, just the strategy.
```

**Verification:**
- [ ] Draw integration diagram
- [ ] Understand data flow
- [ ] Identify compatibility issues
- [ ] Plan testing approach

#### Prompt 3.5: Minimal Integration Code
```
Show me ONLY the shell_bnfc.c skeleton:
- Includes needed
- Main loop structure
- Calling psInput()
- Error handling

Keep it under 50 lines. No execution logic yet.
```

**Verification:**
- [ ] Read every line
- [ ] Understand each include
- [ ] Verify error handling
- [ ] Compile to check syntax
- [ ] Test with echo "exit"

---

### PHASE 4: Fork/Exec Implementation

**Goal:** Replace in-process execution with proper fork/exec

#### Prompt 4.1: Fork/Exec Concept Review
```
Explain the difference between:

CURRENT (wrong):
  cmd_func = find_command("cat");
  cmd_func(argc, argv);

TARGET (correct):
  pid = fork();
  if (pid == 0) execvp("cat", argv);
  waitpid(pid, ...);

Why is the first wrong for a shell?
What problems does fork/exec solve?
What are the steps in the parent?
What are the steps in the child?

Use a timeline diagram.
```

**Verification:**
- [ ] Draw process diagram
- [ ] Understand parent/child split
- [ ] Know what each process does
- [ ] Can explain to someone else

#### Prompt 4.2: Error Handling Strategy
```
In fork/exec code, what errors can occur at each step:
1. fork() fails
2. execvp() fails
3. waitpid() fails
4. child killed by signal

For each, explain:
- Why it might happen
- How to detect it
- What to do about it
- What NOT to do

Don't write code, just explain the strategy.
```

**Verification:**
- [ ] List all possible errors
- [ ] Understand each scenario
- [ ] Know how to test each error
- [ ] Plan error messages

#### Prompt 4.3: Minimal Fork/Exec Function
```
Show me a minimal exec_command_external() function that:
- Takes argv (char **)
- Forks child process
- Child: execvp() with error handling
- Parent: waitpid() and get exit status
- Returns exit status or error

Add DEBUG printf at each step.
Keep it under 40 lines.
```

**Verification:**
- [ ] Understand every line
- [ ] Trace execution for success case
- [ ] Trace execution for failure case
- [ ] Identify all error paths
- [ ] Test with echo, test with bad command

#### Prompt 4.4: Built-in Handling
```
Built-in commands (cd, exit) can't use execvp.
Explain:
1. Why built-ins must run in parent process
2. How to detect if command is built-in
3. Where in the code to check
4. How to preserve existing built-in implementations

Then show me just the is_builtin() function.
```

**Verification:**
- [ ] Understand built-in necessity
- [ ] Can list all built-ins
- [ ] Know where to check
- [ ] Test cd still works

#### Prompt 4.5: BNFC Integration
```
I have:
- BNFC parser that gives me Command* AST
- exec_command_external() function

Show me how to:
1. Extract Word and ListWord from Command*
2. Build argv array (char**)
3. Call exec_command_external()
4. Free memory properly

Show only the conversion function, under 30 lines.
```

**Verification:**
- [ ] Understand AST structure
- [ ] Verify memory allocation
- [ ] Check for leaks
- [ ] Test with multiple args
- [ ] Test with no args

---

### PHASE 5: Pipes Implementation

**Goal:** Support multi-stage pipelines

#### Prompt 5.1: Pipe Mechanism Deep Dive
```
Explain pipes using a concrete example:

echo "hello" | cat | wc -l

1. How many pipes are created?
2. What are the file descriptors?
3. Which process writes to which FD?
4. Which process reads from which FD?
5. When are pipes closed and why?
6. What happens if we don't close?

Use a diagram showing all processes and FDs.
```

**Verification:**
- [ ] Draw the FD diagram
- [ ] Label all pipes
- [ ] Show all dup2() calls
- [ ] Identify all close() calls
- [ ] Can explain to someone

#### Prompt 5.2: Grammar Update
```
My current grammar has:
  Cmd. Command ::= Word [Word] ;

I need to support pipes. Explain:
1. What new nonterminal to add (Pipeline?)
2. How to express "cmd | cmd | cmd"
3. Left vs right recursion for pipes
4. How to update Command to use Pipeline

Show me just the grammar rules, commented.
```

**Verification:**
- [ ] Understand new rules
- [ ] Draw AST for "a | b | c"
- [ ] Verify no ambiguity
- [ ] Regenerate and test parse

#### Prompt 5.3: Pipeline Data Structure Planning
```
For a 3-command pipeline (A | B | C):

Without showing implementation, explain:
1. What data structure to hold all commands?
2. How to iterate through pipeline?
3. How to count stages?
4. When to detect "single command vs pipeline"?

Then show me just the struct or array declaration.
```

**Verification:**
- [ ] Understand data structure choice
- [ ] Can draw memory layout
- [ ] Know iteration pattern

#### Prompt 5.4: Minimal Pipe Creation
```
Show me ONLY the pipe creation loop for N commands:

for (i = 0; i < N-1; i++) {
  pipe(pipes[i]);
}

Explain:
- Why N-1 pipes for N commands?
- How are pipes indexed?
- What is pipes[i][0] vs pipes[i][1]?

Keep explanation under 10 lines.
```

**Verification:**
- [ ] Understand pipe count
- [ ] Know read vs write end
- [ ] Can diagram for 3 commands

#### Prompt 5.5: Single Stage Wiring
```
For the SECOND command in a 3-stage pipeline:
- It reads from pipe[0]
- It writes to pipe[1]

Show me ONLY the dup2() calls for this stage.
Explain each dup2().
```

**Verification:**
- [ ] Understand this stage only
- [ ] Implement for stage 1
- [ ] Implement for stage 3
- [ ] Generalize the pattern

#### Prompt 5.6: Full Pipeline Implementation Strategy
```
I understand:
- Pipe creation
- Single stage wiring
- Close requirements

Now explain the COMPLETE algorithm:
1. Create pipes
2. Fork each stage (in what order?)
3. Wire each stage's FDs
4. Close pipes where?
5. Wait for children (all? in order?)

Use pseudocode, not C.
```

**Verification:**
- [ ] Write algorithm in your own words
- [ ] Identify critical steps
- [ ] Understand why order matters

#### Prompt 5.7: Implementation with Debug Output
```
Using the algorithm we discussed, show me exec_pipeline():
- Take char*** cmds and int num_cmds
- Full implementation
- DEBUG printf at every major step
- Detailed comments

I'll verify it line by line.
```

**Verification:**
- [ ] Read line by line
- [ ] Verify each fork
- [ ] Verify each dup2
- [ ] Verify all closes
- [ ] Test with 2 commands
- [ ] Test with 3 commands

---

### PHASE 6: Redirections Implementation

**Goal:** Support <, >, >> redirections

#### Prompt 6.1: Redirection Concepts
```
Explain file redirections:

echo "hello" > file.txt
cat < input.txt
echo "line2" >> file.txt

For each:
1. What system call opens the file?
2. What flags are used (O_RDONLY, etc)?
3. What FD is dup2'd?
4. When is the file closed?
5. What permissions for creation?

Don't show code, just explain concepts.
```

**Verification:**
- [ ] Understand open() flags
- [ ] Know dup2() target FD
- [ ] Understand file permissions
- [ ] Can explain difference: > vs >>

#### Prompt 6.2: Grammar Update for Redirections
```
Current grammar:
  CmdPipeline. Command ::= Pipeline ;

I need to add redirections. Explain:
1. Should redirection be part of Command or Pipeline?
2. How to support multiple redirections?
3. Should I use a list? Optional?
4. Left or right of command?

Then show just the grammar additions.
```

**Verification:**
- [ ] Understand placement
- [ ] Draw AST for "cat < in.txt > out.txt"
- [ ] Regenerate parser
- [ ] Test parsing only

#### Prompt 6.3: Redirection Data Structure
```
I need to store redirections from BNFC AST.
Suggest:
1. What struct to hold one redirection?
2. What fields (type, filename)?
3. How to represent as array?

Show only the typedef, under 10 lines.
```

**Verification:**
- [ ] Understand structure
- [ ] Can fill from AST
- [ ] Can iterate through array

#### Prompt 6.4: Apply Redirection Logic
```
Show me apply_redirections() that:
- Takes array of redirections
- Called in child process before exec
- Opens files with correct flags
- dup2's to correct FD
- Handles errors

Add a switch case for each type.
Keep under 50 lines with comments.
```

**Verification:**
- [ ] Understand each open() call
- [ ] Verify dup2() targets
- [ ] Check error handling
- [ ] Test > redirection
- [ ] Test < redirection
- [ ] Test >> redirection

#### Prompt 6.5: Integration with Pipeline
```
Redirections should apply to the LAST command in a pipeline:

echo "test" | cat | wc -l > count.txt

Where in my pipeline code should I:
1. Check for redirections?
2. Apply them?
3. Only to which process?

Show me the modified section only.
```

**Verification:**
- [ ] Understand application point
- [ ] Test pipe | pipe > file
- [ ] Test simple command > file
- [ ] Verify only last command affected

---

### PHASE 7: Background Jobs

**Goal:** Support background execution with &

#### Prompt 7.1: Background Job Concepts
```
Explain background jobs:

sleep 10 &
echo "Done"

1. What does parent do differently?
2. How to know when job finishes?
3. What is SIGCHLD?
4. Why do we need signal handler?
5. What is job tracking?

Use timeline diagram.
```

**Verification:**
- [ ] Understand signal handling
- [ ] Know why SIGCHLD exists
- [ ] Can draw process lifetime
- [ ] Understand zombie process issue

#### Prompt 7.2: Grammar for Background
```
Current:
  CmdRedir. Command ::= Pipeline [Redir] ;

Add background support. Explain:
1. Where does & fit?
2. New nonterminal for Job?
3. Job vs Command relationship?

Show grammar modifications only.
```

**Verification:**
- [ ] Understand grammar changes
- [ ] Parse "cmd &"
- [ ] Parse "cmd ; cmd &"
- [ ] Regenerate parser

#### Prompt 7.3: SIGCHLD Handler Strategy
```
Explain SIGCHLD handler requirements:
1. What to do when signal arrives?
2. Why use waitpid with WNOHANG?
3. How to find which job finished?
4. What if multiple children exit?
5. Signal safety concerns?

No code yet, just explain strategy.
```

**Verification:**
- [ ] Understand handler purpose
- [ ] Know WNOHANG meaning
- [ ] Understand reaping all children
- [ ] Know signal-safe functions

#### Prompt 7.4: Job Tracking Structure
```
Show me:
1. Job structure (job_num, pid, command, running)
2. Global job array
3. add_job() function signature
4. list_jobs() function signature

Just declarations, under 20 lines.
```

**Verification:**
- [ ] Understand job structure
- [ ] Know what to track
- [ ] Can add job
- [ ] Can list jobs

#### Prompt 7.5: SIGCHLD Handler Implementation
```
Show me sigchld_handler() that:
- Loops with waitpid(..., WNOHANG)
- Finds completed job in jobs array
- Prints completion message
- Marks job as not running

Keep under 30 lines, signal-safe.
```

**Verification:**
- [ ] Verify WNOHANG usage
- [ ] Check signal safety
- [ ] Test with sleep 2 &
- [ ] Verify job completes

#### Prompt 7.6: Background Execution
```
Show me exec_command_background() that:
- Forks
- Child: execvp (don't wait)
- Parent: add job, return immediately

Under 30 lines.
```

**Verification:**
- [ ] Verify no wait
- [ ] Test returns immediately
- [ ] Job tracked correctly
- [ ] Completion detected

---

## Verification Workflows

### After Every AI Suggestion

```
┌─────────────────────────────────────┐
│ 1. READ the suggestion carefully    │
│ 2. UNDERSTAND each line             │
│ 3. ASK questions if unclear         │
│ 4. PREDICT what it will do          │
│ 5. IMPLEMENT it yourself            │
│ 6. TEST with simple input           │
│ 7. TEST with edge cases             │
│ 8. DOCUMENT what you learned        │
└─────────────────────────────────────┘
```

### Testing Checklist

After implementing AI suggestions:

```bash
# Compilation test
make clean && make
# Expected: No warnings, no errors

# Basic functionality test
echo "simple command" | ./picobox
# Expected: Works as before

# New feature test
echo "test new feature" | ./picobox
# Expected: New feature works

# Error handling test
echo "invalid syntax" | ./picobox
# Expected: Clean error message

# Edge case test
echo "edge case input" | ./picobox
# Expected: Handles gracefully
```

### Code Review Checklist

Before accepting AI code:

- [ ] No undefined behavior
- [ ] All pointers checked for NULL
- [ ] All system calls check return values
- [ ] No memory leaks (verify with valgrind)
- [ ] All file descriptors closed
- [ ] Error messages are helpful
- [ ] Debug output is conditional
- [ ] Comments explain WHY, not WHAT
- [ ] Consistent with existing code style

---

## Common Pitfalls and Solutions

### Pitfall 1: Accepting Code Without Understanding

**Symptom:**
```
YOU: "Implement pipes"
AI: [200 lines of code]
YOU: [copy-paste]
RESULT: Doesn't compile, or compiles but doesn't work
```

**Solution:**
```
1. Ask AI to explain the approach first
2. Request minimal code (one function at a time)
3. Implement core logic yourself
4. Use AI for verification, not generation
```

### Pitfall 2: Not Testing Incrementally

**Symptom:**
```
- Implement 5 features
- Try to test all at once
- Everything broken
- Can't identify which feature causes problem
```

**Solution:**
```
1. Implement ONE feature
2. Test that feature thoroughly
3. Commit working code
4. Only then move to next feature
```

### Pitfall 3: Ignoring Error Cases

**Symptom:**
```
AI shows happy path only
You implement happy path only
Shell crashes on bad input
```

**Solution:**
```
ALWAYS ask AI:
"What can go wrong with this code?"
"What errors should I handle?"
"Show me error handling for each system call"
```

### Pitfall 4: Memory Leaks

**Symptom:**
```
AI allocates memory
AI doesn't show where to free
You don't think about it
valgrind shows leaks
```

**Solution:**
```
For EVERY malloc/strdup:
1. Mark where it's allocated
2. Trace where pointer goes
3. Ensure it's freed on ALL paths
4. Run valgrind frequently
```

### Pitfall 5: Copy-Paste Without Adaptation

**Symptom:**
```
AI gives you code for "similar" problem
You copy-paste
Doesn't quite work for your case
```

**Solution:**
```
1. Understand AI's solution for their problem
2. Identify differences with your problem
3. Adapt the approach, don't copy code
4. Implement solution yourself
```

---

## Prompt Templates

### Template 1: Concept Explanation

```
I need to implement [FEATURE] in my shell.

Before showing code, explain:
1. What problem does [FEATURE] solve?
2. What are the key concepts?
3. What system calls or APIs are involved?
4. What are the edge cases?
5. How does it interact with [EXISTING FEATURE]?

Use simple language and examples.
```

### Template 2: Grammar Design

```
I have this BNFC grammar:

[paste current grammar]

I need to add support for [FEATURE].

First, explain:
1. What new nonterminals are needed?
2. How should they relate to existing rules?
3. Are there ambiguity concerns?

Then show ONLY the new/modified grammar rules with comments.
```

### Template 3: Minimal Implementation

```
I understand the concept of [FEATURE].

Show me ONLY the minimal function to [SPECIFIC TASK]:
- Function signature
- Core logic (no error handling yet)
- Comments explaining each step

Keep it under [N] lines.
I'll add error handling myself.
```

### Template 4: Code Review

```
I implemented [FEATURE]. Here's my code:

[paste your code]

Review it for:
1. Correctness
2. Memory leaks
3. Error handling gaps
4. Edge cases I missed
5. Style issues

Be specific about what to change and why.
```

### Template 5: Debugging Assistance

```
I'm getting this error:

[paste error]

When running:

[paste command]

Here's the relevant code:

[paste code section]

Help me debug:
1. What is the likely cause?
2. What should I check?
3. What debug output would help?
4. How to reproduce minimally?

Don't fix it for me, help me understand it.
```

### Template 6: Test Case Design

```
I implemented [FEATURE].

Help me design test cases:
1. What's the simplest test?
2. What's a complex valid case?
3. What error cases should I test?
4. What edge cases exist?

For each test, specify:
- Input
- Expected output
- Why this test matters
```

---

## Documentation Requirements

### Prompt Log

Keep a file `docs/ai_prompts.md`:

```markdown
# AI Interaction Log

## Phase 3: Grammar Integration

### Prompt 1: Grammar Design (2024-01-15)
**Question:**
[exact prompt]

**AI Response:**
[key points from response]

**What I Did:**
- Implemented X based on suggestion
- Modified Y because of Z reason
- Tested with inputs A, B, C

**Outcome:**
✅ Success / ❌ Failed because...

**Lessons Learned:**
- Thing 1
- Thing 2

---

### Prompt 2: Integration Strategy (2024-01-15)
[same format]
```

### Code Comments

After implementing AI suggestions, add comments:

```c
/*
 * Pipeline execution implementation
 * 
 * Based on AI suggestion to use pipe array and fork loop.
 * Modified to add debug output and better error handling.
 * 
 * Key insight: Must close ALL pipe FDs in each child,
 * not just the ones it's using.
 * 
 * Tested with:
 * - echo "test" | cat
 * - echo "a" | tr a-z A-Z | cat
 * - Bad command in pipeline
 */
int exec_pipeline(char ***cmds, int num_cmds)
{
    // ...
}
```

### Test Documentation

For each test, document:

```bash
# Test: Three-stage pipeline
# Purpose: Verify pipe FD wiring for 3 commands
# Based on: AI suggestion to test increasing complexity
# Input: echo "HELLO" | tr A-Z a-z | cat
# Expected: "hello"
# Edge cases tested: 
# - First command output goes to second
# - Second command output goes to third
# - Final output reaches terminal
# Verified: 2024-01-15
```

---

## Phase-Specific Prompt Guides

### Phase 2 Prompts (Calculator)

```
1. "Explain the calculator grammar precedence rules using examples"
2. "What AST is generated for '1 + 2 * 3' and why?"
3. "How does the visitor pattern work in Skeleton.c?"
4. "Show me minimal code to evaluate one operator"
5. "Help me understand the recursive calls in visitExp"
```

### Phase 3 Prompts (Simple Grammar)

```
1. "Design a minimal shell grammar for 'cmd arg1 arg2'"
2. "Explain BNFC separator directive for command lists"
3. "Show me how to define a Word token with regex"
4. "How do I call the BNFC parser from my main loop?"
5. "Help me convert BNFC AST to argv array"
```

### Phase 4 Prompts (Fork/Exec)

```
1. "Explain fork/exec for shell command execution"
2. "What errors can fork() have and how to handle each?"
3. "Show me minimal fork/exec with error handling"
4. "How to preserve built-in commands when using fork?"
5. "Help me debug: child process not executing"
```

### Phase 5 Prompts (Pipes)

```
1. "Explain pipe file descriptor wiring for 3 commands"
2. "Why do we need N-1 pipes for N commands?"
3. "Show me the dup2 calls for middle pipeline stage"
4. "When and where should I close pipe FDs?"
5. "Help debug: pipeline hangs or shows incomplete output"
```

### Phase 6 Prompts (Redirections)

```
1. "Explain difference between > and >> redirection"
2. "What open() flags for input vs output redirection?"
3. "Show me minimal code to apply one redirection"
4. "How to combine redirections with pipelines?"
5. "Help debug: redirection creates empty file"
```

### Phase 7 Prompts (Background)

```
1. "Explain SIGCHLD and why we need a handler"
2. "Show me minimal signal handler for job completion"
3. "How to track multiple background jobs?"
4. "What's the difference between wait and waitpid WNOHANG?"
5. "Help debug: background job becomes zombie"
```

---

## Anti-Patterns to Avoid

### ❌ Anti-Pattern 1: "Make it work" prompt

```
BAD: "Implement complete pipe support for my shell"
```

Why it's bad:
- Too broad
- You won't understand output
- Likely to have bugs
- Hard to debug

**✅ Better:**
```
GOOD: "Explain the pipe() system call and what file descriptors it creates"
THEN: "Show me how to wire pipes for exactly 2 commands"
THEN: "Now help me generalize to N commands"
```

### ❌ Anti-Pattern 2: Debugging by AI

```
BAD: "This code doesn't work, fix it: [paste 100 lines]"
```

Why it's bad:
- You don't learn root cause
- AI might fix symptom, not problem
- You can't fix next similar issue

**✅ Better:**
```
GOOD: "This pipe code hangs. Help me understand:
1. What causes pipes to hang?
2. How to debug pipe FD state?
3. What to check with lsof?
I'll add debug output and investigate."
```

### ❌ Anti-Pattern 3: Incremental feature creep

```
BAD: "Add pipes. Also add redirections. Also background jobs. Also..."
```

Why it's bad:
- Can't test incrementally
- Interactions between features
- Hard to isolate bugs

**✅ Better:**
```
GOOD: "Just pipes first. I'll test thoroughly. Then ask about redirections separately."
```

### ❌ Anti-Pattern 4: Blind integration

```
BAD: [Copy AI code] → [Paste in project] → [Compile] → [Hope it works]
```

Why it's bad:
- Don't understand code
- Can't maintain it
- Can't explain it

**✅ Better:**
```
GOOD: [Read AI code] → [Understand each line] → [Type it myself] → [Test each part] → [Modify as needed]
```

---

## Success Metrics

### You're Using AI Effectively When:

✅ You can explain AI's suggestions to someone else  
✅ You modify AI code to fit your needs  
✅ You catch errors in AI suggestions  
✅ You test before and after AI changes  
✅ Your prompt log shows learning progression  
✅ You ask "why" more than "how"  
✅ You implement before asking for next feature  
✅ Your code has your style, not AI's  

### You're Using AI Ineffectively When:

❌ You can't explain code you just added  
❌ You're copy-pasting without reading  
❌ Tests fail and you don't know why  
❌ You're asking AI to fix AI-generated bugs  
❌ You skip verification steps  
❌ You move to next phase without passing tests  
❌ Your code is a patchwork of AI snippets  
❌ You're stuck in prompt-paste-fail loop  

---

## Final Checklist

### Before Asking AI

- [ ] Have I tried to understand this myself?
- [ ] Do I have a specific question?
- [ ] Have I defined success criteria?
- [ ] Do I have a test plan?

### When Receiving AI Response

- [ ] Do I understand the explanation?
- [ ] Can I implement this myself?
- [ ] Have I identified potential issues?
- [ ] Do I know how to test it?

### After Implementing

- [ ] Did I test basic case?
- [ ] Did I test error cases?
- [ ] Did I check for memory leaks?
- [ ] Did I document what I learned?
- [ ] Can I explain it to someone?

### Before Moving to Next Phase

- [ ] All tests passing?
- [ ] Code reviewed?
- [ ] No known bugs?
- [ ] Committed to git?
- [ ] Prompt log updated?

---

## Remember

**AI is a tool, not a solution provider.**

Your goal is not to get AI to write your shell.  
Your goal is to learn deeply by using AI to accelerate understanding.

Every prompt should make you smarter.  
Every response should be verified.  
Every implementation should be yours.

Test everything. Understand everything. Document everything.

**You are the developer. AI is the assistant.**

---

## Quick Reference Card

Print this and keep it visible:

```
┌─────────────────────────────────────────────────┐
│          AI-ASSISTED DEVELOPMENT RULES          │
├─────────────────────────────────────────────────┤
│                                                 │
│  1. EXPLAIN before CODE                         │
│  2. MINIMAL changes only                        │
│  3. TEST immediately                            │
│  4. UNDERSTAND every line                       │
│  5. VERIFY with your own eyes                   │
│  6. DOCUMENT what you learned                   │
│  7. NEVER blind copy-paste                      │
│  8. ASK "why" not just "how"                    │
│                                                 │
│  If stuck > 30 min: Ask for explanation         │
│  If code fails: Debug yourself first            │
│  If unsure: Test in isolation                   │
│  If lost: Go back to last working state         │
│                                                 │
└─────────────────────────────────────────────────┘
```

---

**END OF AI AGENT GUIDE**

Use this guide as your companion throughout the BNFC shell project.  
Refer to it before every AI interaction.  
Update it with your own learnings.  
Share it with others who might benefit.

Good luck, and remember: The best AI is the one that makes you a better developer! 🚀