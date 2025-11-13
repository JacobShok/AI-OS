# PicoBox: BusyBox-Style Unix Utilities
   
   ## Project Overview
   Building a single-binary implementation of essential Unix command-line utilities
   in the style of BusyBox. This is for a Systems Programming course.
   
   ## Goals
   - Create 15-30 Unix commands in C
   - Use single binary with symlink dispatch (BusyBox style)
   - Focus on file system manipulation commands
   - Must be production-quality: error handling, help text, proper option parsing
   - Target: embedded Linux systems (eventual ESP32 port possible)
   
   ## Technical Constraints
   - Language: C (C11 standard)
   - Platform: Linux 
   - Compiler: GCC
   - Build: GNU Make
   - Libraries allowed: Standard C library, POSIX APIs
   - External dependencies: Minimize (prefer standard library)
   - on mac book m1 chip
   
   ## Critical Rules
   1. NEVER use exit() in command implementations - always return
   2. ALL commands must handle both file arguments and stdin
   3. ALL commands must implement --help or -h
   4. Human-readable output with -h flag (for sizes, dates)
   5. Proper error handling with perror() and return codes
   6. Memory leaks are unacceptable
   7. Follow Unix philosophy: do one thing well
   8. add test too complex functions so we can debug and test implementations to see if it works
   9. always check to see if a file is already created so you know if to create a new on or not
   
   ## Code Style
   - K&R indentation style
   - 4 spaces (no tabs)
   - Descriptive variable names for clarity
   - Short variable names for counters (i, j, k)
   - Comments for complex logic only
   - Function names: cmd_name() for commands, utility functions separate

   ## Option Parsing Standard

   **MANDATORY:** ALL commands MUST use standard POSIX `getopt()` for option parsing.

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
                   /* ALWAYS validate optarg immediately */
                   if (!option_value || *option_value == '\0') {
                       fprintf(stderr, "example: invalid argument for -n\n");
                       usage_example();
                       return EXIT_ERROR;
                   }
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
       for (int i = optind; i < argc; i++) {
           /* Process argv[i] */
       }

       return EXIT_OK;
   }
   ```

   ### Critical Rules for getopt

   1. **ALWAYS** set `optind = 1` at the start of each command (dispatcher requirement)
   2. **ALWAYS** handle the `'?'` case (invalid options)
   3. **ALWAYS** handle the `'h'` case for help
   4. **ALWAYS** validate all `optarg` values immediately
   5. **USE** `argv[optind]` to access first non-option argument
   6. **NEVER** use `strcmp()` for option parsing
   7. **NEVER** use manual loops to parse options

   ### Option String Format

   - `"abc"` - Three boolean flags: -a, -b, -c
   - `"n:"` - Option -n REQUIRES an argument (in `optarg`)
   - `"abn:h"` - Mix of boolean flags and option with argument

   ### Long Options (--help)

   Check manually before getopt loop:
   ```c
   if (argc > 1 && strcmp(argv[1], "--help") == 0) {
       usage_cmd();
       return EXIT_OK;
   }
   ```

   ### DO NOT

   - ❌ Use `strcmp()` for option parsing
   - ❌ Forget to reset `optind = 1`
   - ❌ Skip validation of `optarg`
   - ❌ Use `exit()` in commands - always return

   See `docs/getopt_migration_plan.md` for detailed examples and patterns.

   ## Project Structure
```
   picobox/
   ├── src/
   │   ├── main.c           # Dispatcher
   │   ├── cmd_ls.c         # ls implementation
   │   ├── cmd_cp.c         # cp implementation
   │   └── ...
   ├── include/
   │   ├── picobox.h        # Main header
   │   └── utils.h          # Utility functions
   ├── build/               # Output directory
   ├── docs/
   │   ├── agent.md         # This file
   │   └── plan.md          # Implementation plan
   └── Makefile
```
   
   ## Implementation Approach
   - Single binary "picobox" contains all commands
   - argv[0] determines which command to run
   - Symbolic links: ls -> picobox, cp -> picobox, etc.
   - Shared utility functions for argument parsing, error handling
   
   ## Current Phase
   Phase 1: Core file system commands 
   
   ## References
   - POSIX.1-2017 standard
   - Linux man pages (man 2 for syscalls, man 3 for library)
   - BusyBox source (for reference only, not copying)