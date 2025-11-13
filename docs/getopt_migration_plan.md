# getopt Migration Plan - Detailed Implementation Guide

## Executive Summary

**Goal:** Migrate all 25 commands from manual strcmp-based option parsing to standard POSIX `getopt()`.

**Approach:** Delete unused `parse_options()` wrapper, use `getopt()` directly in each command.

**Timeline:** 6-8 hours of focused work

**Status:** Planning phase complete, ready for implementation

---

## Phase 1: Documentation & Infrastructure (30 minutes)

### Task 1.1: Update agent.md with getopt guidelines ✓
**File:** `docs/agent.md`

**Add new section after line 40:**

```markdown
## Option Parsing Standard

ALL commands MUST use standard POSIX `getopt()` for option parsing.

### Required Pattern

```c
int cmd_example(int argc, char **argv)
{
    int opt;
    int flag_a = 0;
    int flag_b = 0;
    char *option_value = NULL;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Parse options */
    while ((opt = getopt(argc, argv, "abn:h")) != -1) {
        switch (opt) {
            case 'a':
                flag_a = 1;
                break;
            case 'b':
                flag_b = 1;
                break;
            case 'n':
                option_value = optarg;
                /* Validate optarg here */
                break;
            case 'h':
                usage_example();
                return EXIT_OK;
            case '?':
                /* getopt prints error automatically */
                usage_example();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Non-option arguments start at argv[optind] */
    int first_arg = optind;

    /* Command implementation... */
}
```

### Option String Format

- `"abc"` - three boolean flags: -a, -b, -c
- `"n:"` - option -n requires an argument (available in `optarg`)
- `"n::"` - option -n has optional argument (GNU extension, avoid)
- `"hn:"` - flag -h and option -n with required argument

### Critical Rules

1. **ALWAYS** set `optind = 1` at the start of each command (dispatcher calls multiple commands)
2. **ALWAYS** handle the `'?'` case (invalid options)
3. **ALWAYS** handle the `'h'` case for help
4. **VALIDATE** all `optarg` values immediately in the case statement
5. **USE** `argv[optind]` to access first non-option argument
6. **NEVER** use `strcmp()` for option parsing

### Long Options (--help)

For `--help` support, manually check before getopt loop:

```c
if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    usage_cmd();
    return EXIT_OK;
}
```

Or use `getopt_long()` (GNU extension, available on macOS/Linux):

```c
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

while ((opt = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
    /* ... */
}
```

### DO NOT

- ❌ Use `strcmp()` for option parsing
- ❌ Forget to reset `optind = 1`
- ❌ Parse options with manual loops
- ❌ Skip validation of `optarg`
- ❌ Use global variables for option state
```

---

### Task 1.2: Update plan.md Option Parsing section ✓
**File:** `docs/plan.md`

**Replace lines 1015-1036 (Option Parsing Helper section) with:**

```markdown
### Option Parsing - UPDATED APPROACH

**Decision:** Use POSIX `getopt()` directly in each command instead of wrapper.

**Rationale:**
- Standard approach every C programmer knows
- Handles options with arguments naturally
- More flexible for per-command needs
- Simpler mental model (one approach)
- Less code to maintain (no wrapper)

**Implementation:**
Each command uses this pattern:

```c
int cmd_name(int argc, char **argv)
{
    int opt;
    optind = 1;  /* Reset for dispatcher */

    while ((opt = getopt(argc, argv, "optstring")) != -1) {
        switch (opt) {
            case 'x': /* handle option */ break;
            case 'h': usage_name(); return EXIT_OK;
            case '?': usage_name(); return EXIT_ERROR;
        }
    }

    /* Process argv[optind] onwards */
}
```

See `docs/agent.md` for complete guidelines.
```

---

### Task 1.3: Remove parse_options from utils.h ✓
**File:** `include/utils.h`

**Delete lines 135-160:**
```c
/* ===== Option Parsing Helper ===== */
// ... entire section ...
void free_options(struct options *opts);
```

---

### Task 1.4: Remove parse_options from utils.c ✓
**File:** `src/utils.c`

**Delete lines 272-316:**
```c
/* ===== Option Parsing Helper ===== */
// ... entire section including parse_options and free_options ...
```

---

## Phase 2: Command Migration (5-6 hours)

### Migration Principles

For each command:
1. Read current implementation
2. Identify all options (boolean flags + options with arguments)
3. Create optstring (e.g., "abn:h")
4. Replace strcmp/manual parsing with getopt loop
5. Update usage message if needed
6. Test thoroughly

---

### Group A: Simple Commands (30 minutes)
**Commands with no options except -h/--help**

#### A1: cmd_true.c
**Current:** No options at all
**Migration:** Add minimal getopt for --help support

```c
int cmd_true(int argc, char **argv)
{
    int opt;
    optind = 1;

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_true();
        return EXIT_OK;
    }

    return EXIT_OK;
}
```

**Time:** 5 minutes
**Testing:** `./true`, `./true --help`, `./true -x` (should succeed, ignore unknown)

---

#### A2: cmd_false.c
**Current:** No options
**Migration:** Same as cmd_true.c

**Time:** 5 minutes

---

#### A3: cmd_pwd.c
**Current:** No options in current implementation
**Plan says:** -L, -P options (not implemented yet)

**Migration:** Add getopt ready for future options

```c
int cmd_pwd(int argc, char **argv)
{
    int opt;
    int logical = 0;  /* -L flag (future) */

    optind = 1;

    while ((opt = getopt(argc, argv, "LPh")) != -1) {
        switch (opt) {
            case 'L':
                logical = 1;
                break;
            case 'P':
                logical = 0;
                break;
            case 'h':
                usage_pwd();
                return EXIT_OK;
            case '?':
                usage_pwd();
                return EXIT_ERROR;
        }
    }

    /* getcwd() implementation... */
}
```

**Time:** 10 minutes

---

#### A4: cmd_basename.c
**Current:** Read to check
**Expected:** Takes PATH and optional SUFFIX arguments (not options)

**Migration:** Only --help check needed

**Time:** 5 minutes

---

#### A5: cmd_dirname.c
**Current:** Read to check
**Expected:** Takes PATH argument only

**Migration:** Only --help check needed

**Time:** 5 minutes

---

### Group B: Single Flag Commands (45 minutes)
**Commands with 1-2 boolean options**

#### B1: cmd_echo.c ★ PRIORITY - Example migration
**Current:** Lines 28-63, manual strcmp for -n
**Options:** `-n` (no newline)

**Before:**
```c
/* Check for -n option (no newline) */
if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    print_newline = 0;
    first_arg = 2;
}
```

**After:**
```c
int opt;
int print_newline = 1;

optind = 1;

/* --help check */
if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    usage_echo();
    return EXIT_OK;
}

while ((opt = getopt(argc, argv, "nh")) != -1) {
    switch (opt) {
        case 'n':
            print_newline = 0;
            break;
        case 'h':
            usage_echo();
            return EXIT_OK;
        case '?':
            usage_echo();
            return EXIT_ERROR;
    }
}

/* Print arguments starting at optind */
for (int i = optind; i < argc; i++) {
    if (i > optind) printf(" ");
    printf("%s", argv[i]);
}
```

**Time:** 15 minutes
**Testing:**
```bash
./echo hello world
./echo -n test
./echo -n
./echo -h
./echo --help
./echo -x test  # Should error
```

---

#### B2: cmd_cat.c
**Current:** Lines 86-126, manual loop for -n option
**Options:** `-n` (number lines)

**Migration:**
- Replace lines 94-110 with getopt loop
- Remove `first_file_index` variable
- Use `optind` to find first file argument

**Time:** 15 minutes
**Testing:**
```bash
./cat file.txt
./cat -n file.txt
./cat -n file1.txt file2.txt
./cat < input.txt
./cat -n < input.txt
```

---

#### B3: cmd_mkdir.c
**Current:** Read to check
**Expected:** `-p` (parents), `-m MODE` (mode with argument)

**optstring:** `"pm:h"`

**Key point:** `-m` requires argument, use `optarg`

```c
case 'm':
    /* Parse mode from optarg */
    mode = strtol(optarg, &endptr, 8);
    if (*endptr != '\0') {
        fprintf(stderr, "mkdir: invalid mode '%s'\n", optarg);
        return EXIT_ERROR;
    }
    break;
```

**Time:** 15 minutes

---

#### B4: cmd_touch.c
**Current:** Read to check
**Expected:** `-c` (no create)

**optstring:** `"ch"`

**Time:** 10 minutes

---

### Group C: Multiple Combined Flags (1 hour)
**Commands with 3+ boolean options that can be combined (e.g., -lah)**

#### C1: cmd_ls.c ★ IMPORTANT - Already has custom loop
**Current:** Lines 107-156, custom loop with switch statement
**Options:** `-a`, `-l`, `-h` (all boolean)

**Before (lines 116-141):**
```c
/* Parse options */
while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
    if (strcmp(argv[i], "--help") == 0) {
        usage_ls();
        return EXIT_OK;
    }

    /* Handle combined flags like -lah */
    for (int j = 1; argv[i][j]; j++) {
        switch (argv[i][j]) {
            case 'a': show_all = 1; break;
            case 'l': long_format = 1; break;
            case 'h': human = 1; break;
            default:
                fprintf(stderr, "ls: invalid option '-%c'\n", argv[i][j]);
                usage_ls();
                return EXIT_ERROR;
        }
    }
    i++;
}
```

**After:**
```c
int opt;
int show_all = 0;
int long_format = 0;
int human = 0;

optind = 1;

if (argc > 1 && strcmp(argv[1], "--help") == 0) {
    usage_ls();
    return EXIT_OK;
}

while ((opt = getopt(argc, argv, "alh")) != -1) {
    switch (opt) {
        case 'a':
            show_all = 1;
            break;
        case 'l':
            long_format = 1;
            break;
        case 'h':
            human = 1;
            break;
        case '?':
            usage_ls();
            return EXIT_ERROR;
    }
}

/* Directories start at optind */
if (optind >= argc) {
    return ls_dir(".", show_all, long_format, human);
}

for (int i = optind; i < argc; i++) {
    ls_dir(argv[i], show_all, long_format, human);
}
```

**Benefits:**
- getopt handles combined flags (`-lah`) automatically
- Shorter code (remove inner for loop)
- Better error messages

**Time:** 15 minutes
**Testing:**
```bash
./ls
./ls -l
./ls -a
./ls -lah
./ls -alh
./ls -l -a -h
./ls -h /tmp  # -h for human, not help!
./ls --help
```

---

#### C2: cmd_rm.c
**Expected:** `-r` (recursive), `-f` (force), `-i` (interactive)

**optstring:** `"rfih"`

**Time:** 15 minutes

---

#### C3: cmd_cp.c
**Expected:** `-r` (recursive), `-f` (force), `-p` (preserve)

**optstring:** `"rfph"`

**Time:** 15 minutes

---

#### C4: cmd_mv.c
**Expected:** `-f` (force), `-i` (interactive), `-n` (no clobber)

**optstring:** `"finh"`

**Time:** 15 minutes

---

### Group D: Options with Arguments (1.5 hours)
**Commands with options that take values (use optarg)**

#### D1: cmd_head.c
**Expected:** `-n NUM`, `-c NUM`

**optstring:** `"n:c:h"` (colon means requires argument)

```c
int opt;
int num_lines = 10;  /* default */
int num_bytes = -1;  /* -1 means not set */

optind = 1;

while ((opt = getopt(argc, argv, "n:c:h")) != -1) {
    switch (opt) {
        case 'n':
            num_lines = atoi(optarg);
            if (num_lines <= 0) {
                fprintf(stderr, "head: invalid line count '%s'\n", optarg);
                return EXIT_ERROR;
            }
            break;
        case 'c':
            num_bytes = atoi(optarg);
            if (num_bytes <= 0) {
                fprintf(stderr, "head: invalid byte count '%s'\n", optarg);
                return EXIT_ERROR;
            }
            break;
        case 'h':
            usage_head();
            return EXIT_OK;
        case '?':
            usage_head();
            return EXIT_ERROR;
    }
}

/* Files start at optind */
```

**Key:** Always validate `optarg` immediately!

**Time:** 20 minutes
**Testing:**
```bash
./head file.txt
./head -n 5 file.txt
./head -n5 file.txt       # No space OK
./head -c 100 file.txt
./head -n -10 file.txt    # Should error
./head -n abc file.txt    # Should error
./head -n                 # Should error (missing arg)
```

---

#### D2: cmd_tail.c
**Expected:** `-n NUM`, `-c NUM`, `-f` (follow)

**optstring:** `"n:c:fh"`

**Similar to head, plus boolean -f flag**

**Time:** 20 minutes

---

#### D3: cmd_wc.c
**Expected:** `-l`, `-w`, `-c`, `-m` (all boolean)

**optstring:** `"lwcmh"`

**Time:** 15 minutes

---

#### D4: cmd_chmod.c ★ SPECIAL CASE
**Current:** Lines 21-51, MODE is positional argument, not option
**Expected:** MODE file... with optional `-R` for recursive

**optstring:** `"Rh"`

**Key issue:** MODE comes BEFORE options or AFTER?
- Standard: `chmod MODE file` or `chmod -R MODE file`
- MODE is at `argv[optind]` after option parsing

```c
int opt;
int recursive = 0;

optind = 1;

while ((opt = getopt(argc, argv, "Rh")) != -1) {
    switch (opt) {
        case 'R':
            recursive = 1;
            break;
        case 'h':
            usage_chmod();
            return EXIT_OK;
        case '?':
            usage_chmod();
            return EXIT_ERROR;
    }
}

/* MODE is first non-option arg */
if (optind >= argc) {
    fprintf(stderr, "chmod: missing mode\n");
    usage_chmod();
    return EXIT_ERROR;
}

char *mode_str = argv[optind++];

/* Validate and parse mode */
mode_t mode = /* parse mode_str */;

/* Files start at optind */
if (optind >= argc) {
    fprintf(stderr, "chmod: missing file operand\n");
    usage_chmod();
    return EXIT_ERROR;
}

for (int i = optind; i < argc; i++) {
    if (chmod(argv[i], mode) != 0) {
        perror(argv[i]);
    }
}
```

**Time:** 20 minutes
**Testing:**
```bash
./chmod 755 file.sh
./chmod -R 644 dir/
./chmod 755           # Error: missing file
./chmod               # Error: missing mode
./chmod -h
```

---

#### D5: cmd_ln.c
**Expected:** `-s` (symbolic), `-f` (force)

**optstring:** `"sfh"`

**Time:** 15 minutes

---

### Group E: Complex Commands (2 hours)
**Commands with complex parsing needs**

#### E1: cmd_grep.c
**Expected:** `-i`, `-v`, `-n`, `-r`, `-E` + PATTERN argument

**Complexity:** PATTERN is positional, comes before files

```c
int opt;
int ignore_case = 0;
int invert = 0;
int line_numbers = 0;
int recursive = 0;
int extended = 0;

optind = 1;

while ((opt = getopt(argc, argv, "ivnrEh")) != -1) {
    switch (opt) {
        case 'i': ignore_case = 1; break;
        case 'v': invert = 1; break;
        case 'n': line_numbers = 1; break;
        case 'r': recursive = 1; break;
        case 'E': extended = 1; break;
        case 'h': usage_grep(); return EXIT_OK;
        case '?': usage_grep(); return EXIT_ERROR;
    }
}

/* PATTERN is first non-option arg */
if (optind >= argc) {
    fprintf(stderr, "grep: missing pattern\n");
    return EXIT_ERROR;
}

char *pattern = argv[optind++];

/* Files start at optind (or stdin if none) */
```

**Time:** 30 minutes

---

#### E2: cmd_find.c
**Expected:** `-name PATTERN`, `-type TYPE`, `-size N`, `-mtime N`

**Complexity:** These are NOT traditional flags, they're predicates

**Approach:** Use getopt for simple flags, then manual parse for predicates

```c
int opt;
optind = 1;

while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
        case 'h': usage_find(); return EXIT_OK;
        case '?': return EXIT_ERROR;
    }
}

/* Starting path */
char *start_path = (optind < argc) ? argv[optind++] : ".";

/* Parse predicates manually */
while (optind < argc) {
    if (strcmp(argv[optind], "-name") == 0) {
        /* Next arg is pattern */
        optind++;
        if (optind >= argc) {
            fprintf(stderr, "find: -name requires argument\n");
            return EXIT_ERROR;
        }
        name_pattern = argv[optind++];
    } else if (strcmp(argv[optind], "-type") == 0) {
        /* ... */
    }
}
```

**Reasoning:** find's options don't follow standard flag conventions

**Time:** 45 minutes

---

#### E3: cmd_stat.c, cmd_du.c, cmd_df.c
**Expected:** Various flags, mostly boolean

**Time:** 15 minutes each (45 minutes total)

---

#### E4: cmd_env.c
**Expected:** `-i` flag, then VAR=value pairs, then optional command

**Complexity:** Mixed options and variable assignments

**Time:** 30 minutes

---

#### E5: cmd_sleep.c
**Expected:** Just duration argument with optional suffix

**Migration:** Only --help needed

**Time:** 10 minutes

---

## Phase 3: Testing & Validation (1 hour)

### Test Suite Structure

Create `tests/test_getopt.sh`:

```bash
#!/bin/bash

# Test script for getopt migration
# Tests each command with various option combinations

set -e  # Exit on first error

PICOBOX=./build/picobox

echo "=== Testing getopt migration ==="

# Test echo
echo "Testing echo..."
$PICOBOX echo hello world | grep -q "hello world" || exit 1
$PICOBOX echo -n test | grep -qv $'\n' || exit 1
$PICOBOX echo --help | grep -q "Usage:" || exit 1

# Test cat
echo "Testing cat..."
echo "test" > /tmp/test_cat.txt
$PICOBOX cat /tmp/test_cat.txt | grep -q "test" || exit 1
$PICOBOX cat -n /tmp/test_cat.txt | grep -q "1" || exit 1
rm /tmp/test_cat.txt

# Test ls
echo "Testing ls..."
$PICOBOX ls > /dev/null || exit 1
$PICOBOX ls -l > /dev/null || exit 1
$PICOBOX ls -lah > /dev/null || exit 1

# ... test each command ...

echo "=== All tests passed! ==="
```

### Regression Testing

For each command:
1. Save original behavior: `./old_version cmd args > expected.txt`
2. Run new version: `./new_version cmd args > actual.txt`
3. Compare: `diff expected.txt actual.txt`

---

## Phase 4: Cleanup & Documentation (30 minutes)

### Task 4.1: Update Makefile
Ensure all cmd_*.c files are included in build

### Task 4.2: Run valgrind
Check for memory leaks (getopt shouldn't add any)

```bash
valgrind --leak-check=full ./build/picobox ls -lah
```

### Task 4.3: Final code review
Check each command has:
- [ ] `optind = 1` at start
- [ ] Proper optstring
- [ ] `case '?'` for errors
- [ ] `case 'h'` for help
- [ ] `optarg` validation where needed
- [ ] Uses `argv[optind]` for non-option args

---

## Migration Checklist

### Infrastructure
- [ ] Update agent.md with getopt guidelines
- [ ] Update plan.md Option Parsing section
- [ ] Remove parse_options from utils.h
- [ ] Remove parse_options from utils.c

### Group A (30 min)
- [ ] cmd_true.c
- [ ] cmd_false.c
- [ ] cmd_pwd.c
- [ ] cmd_basename.c
- [ ] cmd_dirname.c

### Group B (45 min)
- [ ] cmd_echo.c ★
- [ ] cmd_cat.c
- [ ] cmd_mkdir.c
- [ ] cmd_touch.c

### Group C (1 hour)
- [ ] cmd_ls.c ★
- [ ] cmd_rm.c
- [ ] cmd_cp.c
- [ ] cmd_mv.c

### Group D (1.5 hours)
- [ ] cmd_head.c
- [ ] cmd_tail.c
- [ ] cmd_wc.c
- [ ] cmd_chmod.c ★
- [ ] cmd_ln.c

### Group E (2 hours)
- [ ] cmd_grep.c
- [ ] cmd_find.c ★
- [ ] cmd_stat.c
- [ ] cmd_du.c
- [ ] cmd_df.c
- [ ] cmd_env.c ★
- [ ] cmd_sleep.c

### Testing
- [ ] Create test_getopt.sh
- [ ] Run regression tests
- [ ] Validate all commands
- [ ] Check memory leaks with valgrind

### Finalization
- [ ] Update Makefile if needed
- [ ] Final code review
- [ ] Update any remaining documentation

---

## Success Criteria

✅ **All 25 commands use getopt**
✅ **Zero strcmp() for option parsing**
✅ **All tests pass**
✅ **No memory leaks**
✅ **Consistent error messages**
✅ **Combined flags work (-lah)**
✅ **Options with arguments validated**
✅ **--help works everywhere**

---

## Timeline Summary

| Phase | Duration | Tasks |
|-------|----------|-------|
| Phase 1: Documentation | 30 min | Update docs, remove parse_options |
| Phase 2: Migration | 5-6 hours | Migrate all 25 commands |
| Phase 3: Testing | 1 hour | Test suite, regression tests |
| Phase 4: Cleanup | 30 min | Final review, documentation |
| **TOTAL** | **7-8 hours** | Complete migration |

---

## Notes

- Start with echo, cat, ls as proof-of-concept
- Test thoroughly after each group
- Can be done incrementally over multiple sessions
- Commands marked with ★ require special attention
- Use `git commit` after each group for safety

---

*Last updated: 2025-11-05*
*Status: Ready for implementation*
