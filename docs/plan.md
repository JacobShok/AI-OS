# PicoBox Implementation Plan - 2 Week Sprint

## Overview
Build 22+ essential Unix commands incrementally, with each day producing working, testable code. Start with infrastructure, then simple file commands, progressing to complex utilities.

---

## **Week 1: Foundation & Core File Commands**

### **Day 1: Build System & Core Infrastructure**
**Goal:** Create the foundation that all commands will build upon

**Tasks:**
1. Create Makefile with incremental build support
2. Implement main.c dispatcher (argv[0] parsing, symlink detection)
3. Build shared utility library (error handling, string utils, path manipulation)
4. Create test framework skeleton

**Deliverable:** Working dispatcher that can route to stub commands

**Files to create:**
- `Makefile` - Build system with clean, all, install targets
- `src/main.c` - Command dispatcher
- `src/utils.c` - Shared utility functions
- `include/picobox.h` - Main header with command prototypes
- `include/utils.h` - Utility function declarations
- `tests/test_utils.c` - Unit tests for utilities

---

### **Day 2-3: First 5 File System Commands**
**Goal:** Implement simple commands to validate architecture

**Commands to implement:**
1. `echo` - Print arguments to stdout
2. `pwd` - Print working directory
3. `cat` - Concatenate files to stdout
4. `mkdir` - Create directories
5. `touch` - Create empty file or update timestamp

**Deliverable:** 5 working commands with --help support and tests

---

### **Day 4-5: File Operations (Part 1)**
**Goal:** Core file manipulation

**Commands to implement:**
6. `ls` - List directory contents
7. `cp` - Copy files/directories
8. `rm` - Remove files/directories
9. `mv` - Move/rename files

**Deliverable:** Core file manipulation working with recursive support

---

### **Day 6-7: File Operations (Part 2) & Text Processing**
**Goal:** Complete basic file operations and add text processing

**Commands to implement:**
10. `head` - Output first N lines
11. `tail` - Output last N lines
12. `wc` - Count lines, words, bytes
13. `ln` - Create hard/symbolic links

**Deliverable:** 13 commands complete, comprehensive tests

---

## **Week 2: Advanced Commands & System Utilities**

### **Day 8-9: Search & Pattern Matching**
**Goal:** Implement search utilities

**Commands to implement:**
14. `grep` - Search text using patterns
15. `find` - Search for files in directory hierarchy
16. `basename` - Strip directory from pathname
17. `dirname` - Extract directory from pathname

**Deliverable:** Search utilities working with basic regex

---

### **Day 10-11: File Permissions & System Info**
**Goal:** File metadata and permissions

**Commands to implement:**
18. `chmod` - Change file permissions
19. `stat` - Display file status
20. `du` - Estimate file space usage
21. `df` - Report file system disk space usage

**Deliverable:** 21 commands complete

---

### **Day 12-13: Process & Environment + Extras**
**Goal:** Environment and simple process control

**Commands to implement:**
22. `env` - Display environment variables
23. `sleep` - Delay for specified time
24. `true` - Return success (exit 0)
25. `false` - Return failure (exit 1)

**Bonus commands if time permits:**
- `cut` - Remove sections from lines
- `sort` - Sort lines of text
- `uniq` - Report or omit repeated lines

**Deliverable:** 25+ commands, full suite

---

### **Day 14: Polish, Testing & Documentation**
**Goal:** Production-ready release

**Tasks:**
1. Comprehensive test suite for all commands
2. Memory leak checking with valgrind
3. Performance optimization
4. Create symlinks for all commands
5. Write user documentation (README.md)
6. Create installation script

**Deliverable:** Production-ready PicoBox

---

## Detailed Command Specifications

### **1. echo**
**What it does:** Print arguments to stdout with optional newline

**Essential options:**
- `-n` - Don't output trailing newline

**System calls/APIs:**
- `write()` or `printf()`

**Input/output:**
- Input: Command-line arguments
- Output: Arguments printed to stdout

**Error cases:**
- None (echo always succeeds)

**Testing:**
```bash
./echo "hello world"           # hello world\n
./echo -n "no newline"         # no newline (no \n)
./echo                         # \n
./echo "multiple" "args"       # multiple args\n
```

---

### **2. pwd**
**What it does:** Print current working directory

**Essential options:**
- `-L` - Use PWD from environment (logical)
- `-P` - Print physical directory (resolve symlinks)

**System calls/APIs:**
- `getcwd()`
- `getenv("PWD")`

**Input/output:**
- Input: None (uses current process state)
- Output: Absolute path to stdout

**Error cases:**
- Directory deleted/inaccessible
- Path too long for buffer

**Testing:**
```bash
./pwd                          # /current/directory
cd /tmp && ../picobox/pwd      # /tmp
```

---

### **3. cat**
**What it does:** Concatenate files and print to stdout

**Essential options:**
- `-n` - Number all output lines
- No args - read from stdin

**System calls/APIs:**
- `open()`, `read()`, `write()`, `close()`
- `fopen()`, `fread()`, `fwrite()`, `fclose()`

**Input/output:**
- Input: Files or stdin
- Output: File contents to stdout

**Error cases:**
- File doesn't exist
- Permission denied
- Is a directory

**Testing:**
```bash
./cat file.txt                 # contents of file.txt
./cat file1.txt file2.txt      # concatenated
./cat < input.txt              # from stdin
./cat -n file.txt              # with line numbers
```

---

### **4. mkdir**
**What it does:** Create directories

**Essential options:**
- `-p` - Create parent directories as needed
- `-m MODE` - Set permission mode

**System calls/APIs:**
- `mkdir()`
- `stat()` to check existence

**Input/output:**
- Input: Directory path(s)
- Output: None (creates directories)

**Error cases:**
- Directory already exists
- Permission denied
- Parent doesn't exist (without -p)

**Testing:**
```bash
./mkdir newdir                 # create newdir/
./mkdir -p a/b/c               # create all
./mkdir existing               # error: exists
```

---

### **5. touch**
**What it does:** Create empty file or update timestamp

**Essential options:**
- `-c` - Don't create file if it doesn't exist
- `-t TIME` - Use specified time instead of now

**System calls/APIs:**
- `open()` with `O_CREAT`
- `utimes()` or `utimensat()` for timestamp

**Input/output:**
- Input: File path(s)
- Output: None (creates/updates files)

**Error cases:**
- Permission denied
- Invalid path

**Testing:**
```bash
./touch newfile.txt            # creates empty file
./touch existing.txt           # updates timestamp
stat -c %Y file.txt            # check timestamp
```

---

### **6. ls**
**What it does:** List directory contents

**Essential options:**
- `-l` - Long format
- `-a` - Show hidden files
- `-h` - Human-readable sizes
- `-R` - Recursive
- `-t` - Sort by modification time

**System calls/APIs:**
- `opendir()`, `readdir()`, `closedir()`
- `stat()`, `lstat()` for file info
- `getpwuid()`, `getgrgid()` for user/group names

**Input/output:**
- Input: Directory path(s) or current dir
- Output: File listing to stdout

**Error cases:**
- Directory doesn't exist
- Permission denied
- Not a directory

**Testing:**
```bash
./ls                           # current directory
./ls -la /tmp                  # long format, all files
./ls -lh                       # human-readable sizes
./ls nonexistent               # error
```

---

### **7. cp**
**What it does:** Copy files and directories

**Essential options:**
- `-r` or `-R` - Copy directories recursively
- `-f` - Force (overwrite without prompt)
- `-p` - Preserve mode, ownership, timestamps

**System calls/APIs:**
- `open()`, `read()`, `write()`, `close()`
- `stat()` to check source/dest
- `opendir()`, `readdir()` for recursive
- `chmod()`, `chown()` for -p

**Input/output:**
- Input: Source and destination paths
- Output: None (creates copies)

**Error cases:**
- Source doesn't exist
- Permission denied
- Dest is directory but source isn't
- Copying directory without -r

**Testing:**
```bash
./cp source.txt dest.txt       # copy file
./cp -r srcdir destdir         # recursive
./cp file.txt /tmp/            # to directory
```

---

### **8. rm**
**What it does:** Remove files or directories

**Essential options:**
- `-r` or `-R` - Remove directories recursively
- `-f` - Force (ignore nonexistent, no prompt)
- `-i` - Interactive prompt

**System calls/APIs:**
- `unlink()` for files
- `rmdir()` for directories
- `opendir()`, `readdir()` for recursive

**Input/output:**
- Input: File/directory path(s)
- Output: None (removes files)

**Error cases:**
- File doesn't exist (unless -f)
- Permission denied
- Directory not empty (without -r)

**Testing:**
```bash
./rm file.txt                  # remove file
./rm -r directory/             # recursive
./rm -f nonexistent            # no error
./rm directory/                # error: is a directory
```

---

### **9. mv**
**What it does:** Move or rename files

**Essential options:**
- `-f` - Force overwrite
- `-i` - Interactive prompt
- `-n` - No overwrite

**System calls/APIs:**
- `rename()` for same filesystem
- `link()` + `unlink()` or copy+delete for cross-fs

**Input/output:**
- Input: Source and destination
- Output: None (moves files)

**Error cases:**
- Source doesn't exist
- Permission denied
- Cross-filesystem move

**Testing:**
```bash
./mv old.txt new.txt           # rename
./mv file.txt /tmp/            # move to directory
./mv dir1 dir2                 # rename directory
```

---

### **10. head**
**What it does:** Output first N lines of files

**Essential options:**
- `-n NUM` - Output first NUM lines (default 10)
- `-c NUM` - Output first NUM bytes

**System calls/APIs:**
- `open()`, `read()`, `close()`
- `fgets()` for line reading

**Input/output:**
- Input: Files or stdin
- Output: First N lines to stdout

**Error cases:**
- File doesn't exist
- Permission denied

**Testing:**
```bash
./head file.txt                # first 10 lines
./head -n 5 file.txt           # first 5 lines
./head -c 100 file.txt         # first 100 bytes
cat file.txt | ./head          # from stdin
```

---

### **11. tail**
**What it does:** Output last N lines of files

**Essential options:**
- `-n NUM` - Output last NUM lines (default 10)
- `-c NUM` - Output last NUM bytes
- `-f` - Follow (output appended data)

**System calls/APIs:**
- `open()`, `read()`, `lseek()`, `close()`
- For -f: loop with `read()` and `sleep()`

**Input/output:**
- Input: Files or stdin
- Output: Last N lines to stdout

**Error cases:**
- File doesn't exist
- Permission denied

**Testing:**
```bash
./tail file.txt                # last 10 lines
./tail -n 5 file.txt           # last 5 lines
./tail -f logfile              # follow mode
```

---

### **12. wc**
**What it does:** Count lines, words, and bytes

**Essential options:**
- `-l` - Count lines only
- `-w` - Count words only
- `-c` - Count bytes only
- `-m` - Count characters

**System calls/APIs:**
- `open()`, `read()`, `close()`
- Character classification from `ctype.h`

**Input/output:**
- Input: Files or stdin
- Output: Counts to stdout

**Error cases:**
- File doesn't exist
- Permission denied

**Testing:**
```bash
./wc file.txt                  # lines words bytes file.txt
./wc -l file.txt               # lines only
echo "hello world" | ./wc -w   # 2
```

---

### **13. ln**
**What it does:** Create hard or symbolic links

**Essential options:**
- `-s` - Create symbolic link
- `-f` - Force (remove existing dest)

**System calls/APIs:**
- `link()` for hard links
- `symlink()` for symbolic links
- `unlink()` for -f

**Input/output:**
- Input: Target and link name
- Output: None (creates link)

**Error cases:**
- Target doesn't exist (hard link)
- Link already exists (without -f)
- Cross-filesystem hard link

**Testing:**
```bash
./ln file.txt hardlink         # hard link
./ln -s file.txt symlink       # symbolic link
ls -li file.txt hardlink       # same inode
readlink symlink               # file.txt
```

---

### **14. grep**
**What it does:** Search for patterns in files

**Essential options:**
- `-i` - Case insensitive
- `-v` - Invert match (non-matching lines)
- `-n` - Show line numbers
- `-r` - Recursive directory search
- `-E` - Extended regex

**System calls/APIs:**
- `open()`, `read()`, `close()`
- `regcomp()`, `regexec()` from `regex.h`
- `opendir()`, `readdir()` for -r

**Input/output:**
- Input: Pattern and files or stdin
- Output: Matching lines to stdout

**Error cases:**
- File doesn't exist
- Invalid regex pattern

**Testing:**
```bash
./grep "pattern" file.txt      # matching lines
./grep -i "HELLO" file.txt     # case insensitive
./grep -n "error" log.txt      # with line numbers
./grep -r "TODO" src/          # recursive
```

---

### **15. find**
**What it does:** Search for files in directory hierarchy

**Essential options:**
- `-name PATTERN` - Match filename
- `-type TYPE` - Match file type (f/d/l)
- `-size N` - Match file size
- `-mtime N` - Modified N days ago

**System calls/APIs:**
- `opendir()`, `readdir()`, `closedir()`
- `stat()`, `lstat()` for file info
- `fnmatch()` for pattern matching

**Input/output:**
- Input: Starting directory and search criteria
- Output: Matching paths to stdout

**Error cases:**
- Starting directory doesn't exist
- Permission denied on subdirectories

**Testing:**
```bash
./find . -name "*.c"           # all C files
./find /tmp -type d            # directories only
./find . -size +1M             # files > 1MB
```

---

### **16. basename**
**What it does:** Strip directory from pathname

**Essential options:**
- `SUFFIX` - Remove trailing suffix

**System calls/APIs:**
- String manipulation only
- `basename()` from `libgen.h` (or implement)

**Input/output:**
- Input: Path string
- Output: Filename to stdout

**Error cases:**
- None (always succeeds)

**Testing:**
```bash
./basename /path/to/file.txt   # file.txt
./basename /path/to/file.txt .txt  # file
./basename /path/to/           # to
```

---

### **17. dirname**
**What it does:** Extract directory from pathname

**Essential options:**
- None

**System calls/APIs:**
- String manipulation
- `dirname()` from `libgen.h` (or implement)

**Input/output:**
- Input: Path string
- Output: Directory path to stdout

**Error cases:**
- None (always succeeds)

**Testing:**
```bash
./dirname /path/to/file.txt    # /path/to
./dirname /path/to/            # /path
./dirname file.txt             # .
```

---

### **18. chmod**
**What it does:** Change file permissions

**Essential options:**
- `-R` - Recursive
- Modes: octal (755) or symbolic (u+x)

**System calls/APIs:**
- `chmod()`
- `stat()` for existing permissions
- `opendir()`, `readdir()` for -R

**Input/output:**
- Input: Mode and file path(s)
- Output: None (changes permissions)

**Error cases:**
- File doesn't exist
- Permission denied
- Invalid mode

**Testing:**
```bash
./chmod 755 file.sh            # rwxr-xr-x
./chmod u+x file.sh            # add execute
./chmod -R 644 dir/            # recursive
stat -c %a file.sh             # check perms
```

---

### **19. stat**
**What it does:** Display file or filesystem status

**Essential options:**
- `-f` - Display filesystem status
- `-L` - Follow symbolic links
- `-t` - Terse format

**System calls/APIs:**
- `stat()`, `lstat()`
- `statfs()` or `statvfs()` for -f

**Input/output:**
- Input: File path(s)
- Output: File metadata to stdout

**Error cases:**
- File doesn't exist
- Permission denied

**Testing:**
```bash
./stat file.txt                # full info
./stat -L symlink              # follow link
./stat -f /                    # filesystem info
```

---

### **20. du**
**What it does:** Estimate file space usage

**Essential options:**
- `-h` - Human-readable sizes
- `-s` - Summary only (don't show subdirs)
- `-a` - All files, not just directories

**System calls/APIs:**
- `opendir()`, `readdir()`, `closedir()`
- `stat()`, `lstat()` for file sizes

**Input/output:**
- Input: File/directory path(s)
- Output: Disk usage to stdout

**Error cases:**
- Path doesn't exist
- Permission denied

**Testing:**
```bash
./du /tmp                      # total with subdirs
./du -sh /tmp                  # summary, human
./du -ah /tmp                  # all files, human
```

---

### **21. df**
**What it does:** Report filesystem disk space usage

**Essential options:**
- `-h` - Human-readable sizes
- `-T` - Print filesystem type

**System calls/APIs:**
- `getmntent()` to read /etc/mtab or /proc/mounts
- `statvfs()` for filesystem stats

**Input/output:**
- Input: Filesystem or directory (optional)
- Output: Space usage table to stdout

**Error cases:**
- Invalid path
- Cannot read mount table

**Testing:**
```bash
./df                           # all filesystems
./df -h                        # human-readable
./df /tmp                      # specific fs
```

---

### **22. env**
**What it does:** Display environment variables or run program with modified environment

**Essential options:**
- `-i` - Ignore inherited environment
- `VAR=value` - Set variable

**System calls/APIs:**
- `environ` global variable
- `getenv()`, `setenv()`, `unsetenv()`
- `execvp()` for running programs

**Input/output:**
- Input: Optional variable assignments and command
- Output: Environment variables or command output

**Error cases:**
- Invalid variable syntax
- Command not found

**Testing:**
```bash
./env                          # print all vars
./env PATH=/bin ls             # run with modified PATH
./env -i                       # empty environment
```

---

### **23. sleep**
**What it does:** Delay for specified time

**Essential options:**
- Suffixes: s (seconds), m (minutes), h (hours)

**System calls/APIs:**
- `sleep()` or `nanosleep()`

**Input/output:**
- Input: Duration
- Output: None (just delays)

**Error cases:**
- Invalid duration
- Negative duration

**Testing:**
```bash
time ./sleep 2                 # delay ~2 seconds
./sleep 0.5                    # 500ms (if supported)
./sleep -1                     # error
```

---

### **24. true**
**What it does:** Return success exit status

**Essential options:**
- None

**System calls/APIs:**
- None

**Input/output:**
- Input: None
- Output: None

**Error cases:**
- None (always succeeds)

**Testing:**
```bash
./true && echo "success"       # success
echo $?                        # 0
```

---

### **25. false**
**What it does:** Return failure exit status

**Essential options:**
- None

**System calls/APIs:**
- None

**Input/output:**
- Input: None
- Output: None

**Error cases:**
- None (always fails)

**Testing:**
```bash
./false || echo "failed"       # failed
echo $?                        # 1
```

---

## Shared Utility Functions (utils.c/utils.h)

### Error Handling Functions

#### `void error_msg(const char *msg)`
Print error message to stderr

**System calls:** `write()` or `fprintf()`

#### `int perror_return(const char *msg, int code)`
Print error with perror() and return error code

**System calls:** `perror()`

#### `void usage(const char *cmd, const char *msg)`
Print usage message for command

**System calls:** `printf()` or `write()`

---

### String Utilities

#### `int str_ends_with(const char *str, const char *suffix)`
Check if string ends with suffix

**Returns:** 1 if true, 0 if false

**Testing:**
```c
assert(str_ends_with("file.txt", ".txt") == 1);
assert(str_ends_with("file.txt", ".c") == 0);
```

#### `int str_starts_with(const char *str, const char *prefix)`
Check if string starts with prefix

**Returns:** 1 if true, 0 if false

#### `char *trim_whitespace(char *str)`
Remove leading and trailing whitespace (modifies in place)

**Returns:** Pointer to trimmed string

**Testing:**
```c
char buf[] = "  hello  ";
assert(strcmp(trim_whitespace(buf), "hello") == 0);
```

---

### Path Manipulation

#### `char *path_join(const char *base, const char *name)`
Safely join two path components

**Returns:** Newly allocated string (caller must free)

**System calls:** `malloc()`

**Testing:**
```c
char *p = path_join("/usr", "bin");
assert(strcmp(p, "/usr/bin") == 0);
free(p);
```

#### `char *get_basename(const char *path)`
Extract filename from path (allocates new string)

**Returns:** Newly allocated string

**Testing:**
```c
char *b = get_basename("/path/to/file.txt");
assert(strcmp(b, "file.txt") == 0);
free(b);
```

#### `char *get_dirname(const char *path)`
Extract directory from path (allocates new string)

**Returns:** Newly allocated string

**Testing:**
```c
char *d = get_dirname("/path/to/file.txt");
assert(strcmp(d, "/path/to") == 0);
free(d);
```

---

### File Utilities

#### `int is_directory(const char *path)`
Check if path is a directory

**System calls:** `stat()`

**Returns:** 1 if directory, 0 otherwise

**Error handling:** Returns 0 on stat() failure

#### `int is_regular_file(const char *path)`
Check if path is a regular file

**System calls:** `stat()`

**Returns:** 1 if regular file, 0 otherwise

#### `int file_exists(const char *path)`
Check if file exists

**System calls:** `access()` or `stat()`

**Returns:** 1 if exists, 0 otherwise

#### `ssize_t copy_file(const char *src, const char *dest)`
Copy file from src to dest

**System calls:** `open()`, `read()`, `write()`, `close()`

**Returns:** Number of bytes copied, or -1 on error

**Error handling:** Sets errno appropriately

**Testing:**
```c
assert(copy_file("src.txt", "dest.txt") > 0);
assert(file_exists("dest.txt"));
```

---

### Human-Readable Formatting

#### `char *format_size(off_t size, char *buf, size_t bufsize)`
Format file size in human-readable format (B, K, M, G, T)

**Example:** 1536 � "1.5K"

**Testing:**
```c
char buf[32];
format_size(1536, buf, sizeof(buf));
assert(strcmp(buf, "1.5K") == 0);
```

#### `char *format_time(time_t t, char *buf, size_t bufsize)`
Format timestamp in readable format

**Example:** "Jan 15 14:30" or "2024-01-15 14:30:00"

---

### Option Parsing - UPDATED APPROACH ⚠️

**Decision:** Use POSIX `getopt()` directly in each command instead of wrapper.

**Previous approach:** A `parse_options()` wrapper was implemented but never adopted by commands.

**Rationale for change:**
- Standard approach every C programmer knows
- Handles options with arguments naturally (via `optarg`)
- More flexible for per-command needs
- Simpler mental model (one approach, not two)
- Less code to maintain (no wrapper infrastructure)
- Commands were already using strcmp instead of wrapper

**Standard Implementation Pattern:**

Each command now uses this pattern:

```c
int cmd_name(int argc, char **argv)
{
    int opt;
    int some_flag = 0;
    char *option_value = NULL;

    /* CRITICAL: Reset getopt for dispatcher */
    optind = 1;

    /* Optional: Check --help before getopt */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage_name();
        return EXIT_OK;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, "fn:h")) != -1) {
        switch (opt) {
            case 'f':
                some_flag = 1;
                break;
            case 'n':
                option_value = optarg;
                /* Validate immediately! */
                if (atoi(optarg) <= 0) {
                    fprintf(stderr, "cmd: invalid value '%s'\n", optarg);
                    return EXIT_ERROR;
                }
                break;
            case 'h':
                usage_name();
                return EXIT_OK;
            case '?':
                /* getopt prints error */
                usage_name();
                return EXIT_ERROR;
            default:
                return EXIT_ERROR;
        }
    }

    /* Non-option arguments start at argv[optind] */
    for (int i = optind; i < argc; i++) {
        /* Process argv[i] */
    }

    return EXIT_OK;
}
```

**Option String Format:**
- `"abc"` - Boolean flags -a, -b, -c
- `"n:"` - Option -n requires argument (available in `optarg`)
- `"abn:h"` - Mix of flags and options with arguments

**Key Rules:**
1. Always reset `optind = 1` at start of command
2. Always validate `optarg` immediately
3. Handle `'?'` case for invalid options
4. Use `argv[optind]` to access first non-option argument

See `docs/agent.md` for complete guidelines and `docs/getopt_migration_plan.md` for migration details.

---

## Key System Calls & APIs Reference

### File I/O (Unbuffered)
- `int open(const char *path, int flags, mode_t mode)`
- `ssize_t read(int fd, void *buf, size_t count)`
- `ssize_t write(int fd, const void *buf, size_t count)`
- `int close(int fd)`
- `off_t lseek(int fd, off_t offset, int whence)`

### File I/O (Buffered)
- `FILE *fopen(const char *path, const char *mode)`
- `size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)`
- `size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)`
- `char *fgets(char *s, int size, FILE *stream)`
- `int fclose(FILE *stream)`

### File Metadata
- `int stat(const char *path, struct stat *buf)`
- `int lstat(const char *path, struct stat *buf)` - Don't follow symlinks
- `int fstat(int fd, struct stat *buf)`

### Directory Operations
- `DIR *opendir(const char *name)`
- `struct dirent *readdir(DIR *dirp)`
- `int closedir(DIR *dirp)`
- `int mkdir(const char *path, mode_t mode)`
- `int rmdir(const char *path)`

### File Operations
- `int unlink(const char *path)` - Delete file
- `int rename(const char *old, const char *new)` - Rename/move
- `int link(const char *old, const char *new)` - Create hard link
- `int symlink(const char *target, const char *linkpath)` - Create symlink
- `int chmod(const char *path, mode_t mode)` - Change permissions
- `int chown(const char *path, uid_t owner, gid_t group)` - Change ownership

### Time Functions
- `int utimes(const char *filename, const struct timeval times[2])` - Set timestamps
- `time_t time(time_t *tloc)` - Get current time
- `struct tm *localtime(const time_t *timep)` - Convert to local time

### Path Manipulation
- `char *getcwd(char *buf, size_t size)` - Get current directory
- `char *realpath(const char *path, char *resolved)` - Resolve to absolute path

### Pattern Matching
- `int fnmatch(const char *pattern, const char *string, int flags)` - Filename matching
- `int regcomp(regex_t *preg, const char *regex, int cflags)` - Compile regex
- `int regexec(const regex_t *preg, const char *string, ...)` - Execute regex

### Process & Environment
- `char *getenv(const char *name)` - Get environment variable
- `int setenv(const char *name, const char *value, int overwrite)` - Set env var
- `extern char **environ` - Environment array
- `unsigned int sleep(unsigned int seconds)` - Sleep

### User/Group Info
- `struct passwd *getpwuid(uid_t uid)` - Get user info
- `struct group *getgrgid(gid_t gid)` - Get group info

---

## Implementation Guidelines

### Memory Management Rules
1. **Always free allocated memory** - Use valgrind to check
2. **Close file descriptors** - Track all open() calls
3. **Close directory streams** - Every opendir() needs closedir()
4. **Free regex compiled patterns** - regfree() after regcomp()

### Error Handling Pattern
```c
int cmd_foo(int argc, char **argv)
{
    int ret = 0;
    FILE *fp = NULL;

    fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return 1;
    }

    // ... do work ...

cleanup:
    if (fp) fclose(fp);
    return ret;
}
```

### Help Text Format
```c
static void usage_foo(void)
{
    fprintf(stderr,
        "Usage: foo [OPTIONS] FILE...\n"
        "Brief description of what foo does.\n"
        "\n"
        "Options:\n"
        "  -a          description of -a\n"
        "  -b          description of -b\n"
        "  -h, --help  display this help and exit\n");
}
```

### Testing Checklist for Each Command
- [ ] Normal operation with files
- [ ] Reading from stdin when applicable
- [ ] Each option flag works correctly
- [ ] Option combinations work together
- [ ] Error: file doesn't exist
- [ ] Error: permission denied
- [ ] Error: invalid option
- [ ] Help text displays correctly
- [ ] No memory leaks (valgrind)
- [ ] Works with empty files
- [ ] Works with large files (> 1MB)
- [ ] Works with binary files (when applicable)
- [ ] Symlink handling (when applicable)

---

## Build System (Makefile)

### Key Targets
- `all` - Build everything
- `clean` - Remove build artifacts
- `test` - Run all tests
- `install` - Install binary and create symlinks
- `uninstall` - Remove installed files
- `valgrind` - Run with memory leak detection

### Compilation Flags
```makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2
LDFLAGS =
INCLUDES = -Iinclude
```

### Structure
```makefile
SRCS = src/main.c src/utils.c src/cmd_echo.c src/cmd_pwd.c ...
OBJS = $(SRCS:.c=.o)
TARGET = build/picobox

$(TARGET): $(OBJS)
    $(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
    $(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
```

---

## Progress Tracking

### Day 1 Checklist
- [ ] Makefile created and working
- [ ] main.c dispatcher implemented
- [ ] utils.c with basic functions
- [ ] Headers (picobox.h, utils.h)
- [ ] Test framework skeleton
- [ ] Can compile and run (even with stub commands)

### Day 2-3 Checklist
- [ ] echo command complete
- [ ] pwd command complete
- [ ] cat command complete
- [ ] mkdir command complete
- [ ] touch command complete
- [ ] All 5 commands have tests
- [ ] All tests passing

### Day 4-5 Checklist
- [ ] ls command with -l, -a, -h, -R
- [ ] cp command with -r, -p
- [ ] rm command with -r, -f
- [ ] mv command complete
- [ ] Integration tests for file operations
- [ ] 9 commands total

### Day 6-7 Checklist
- [ ] head command with -n, -c
- [ ] tail command with -n, -c
- [ ] wc command with -l, -w, -c
- [ ] ln command with -s
- [ ] 13 commands complete
- [ ] Comprehensive test suite

### Day 8-9 Checklist
- [ ] grep with regex support
- [ ] find with -name, -type
- [ ] basename command
- [ ] dirname command
- [ ] 17 commands complete

### Day 10-11 Checklist
- [ ] chmod command
- [ ] stat command
- [ ] du command
- [ ] df command (if time permits)
- [ ] 21 commands complete

### Day 12-13 Checklist
- [ ] env command
- [ ] sleep command
- [ ] true command
- [ ] false command
- [ ] 25+ commands complete
- [ ] All bonus commands implemented

### Day 14 Checklist
- [ ] All memory leaks fixed
- [ ] All tests passing
- [ ] Symlinks created
- [ ] Installation script
- [ ] User documentation
- [ ] README.md complete
- [ ] Final validation

---

## Success Metrics

### Code Quality
- **Zero memory leaks** - Valgrind clean
- **All warnings resolved** - Compile with -Werror
- **Test coverage > 90%** - Every command has tests
- **POSIX compliance** - Matches standard behavior

### Functionality
- **All 22+ commands working** - Basic functionality complete
- **Essential options implemented** - Per spec above
- **Error handling robust** - Handles all error cases
- **Help text complete** - Every command has -h

### User Experience
- **Single binary** - Symlink dispatch works
- **Fast execution** - No unnecessary overhead
- **Clear error messages** - User knows what went wrong
- **Documentation complete** - README, man pages (optional)

---

## Optional Enhancements (If Time Permits)

### Additional Commands
- `cut` - Extract columns
- `sort` - Sort lines
- `uniq` - Remove duplicates
- `tee` - Read from stdin and write to file and stdout
- `seq` - Generate sequences of numbers
- `yes` - Repeatedly output a string
- `whoami` - Print current user
- `groups` - Print group memberships
- `id` - Print user/group IDs
- `date` - Display current date/time

### Advanced Features
- Color output for ls
- Follow mode for tail (-f)
- Extended regex for grep (-E, -P)
- Parallel execution for find
- Progress indicators for long operations
- Configuration file support

---

## Resources & References

### Documentation
- **POSIX Standard:** https://pubs.opengroup.org/onlinepubs/9699919799/
- **Linux man pages:** man 2 (syscalls), man 3 (library)
- **GNU Coreutils source:** For reference implementation

### Testing
- **Valgrind:** Memory leak detection
- **ShellCheck:** Shell script validation (for test scripts)
- **AFL:** Fuzzing (advanced)

### Build Tools
- **Make:** GNU Make documentation
- **GCC:** Compiler flags and options
- **GDB:** Debugging

---

## Notes

This plan prioritizes:
1. **Working code at each step** - Never leave broken code
2. **Testing from day 1** - Catch issues early
3. **Simple to complex** - Build foundation first
4. **File operations first** - Most useful commands
5. **Incremental delivery** - Each day adds value

The goal is a production-quality implementation that demonstrates mastery of:
- C programming
- POSIX system calls
- Unix command-line tools
- Software engineering practices

Let's build something excellent! =�
