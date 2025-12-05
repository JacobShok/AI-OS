/*
 * shell_bnfc.c - BNFC-powered shell implementation
 *
 * This shell uses BNFC-generated parser instead of manual parsing.
 * Phase 3: Grammar-based parsing with in-process execution
 * Phase 4: Fork/exec for proper process isolation
 * Phase 5: Pipes (cmd1 | cmd2 | cmd3)
 * Phase 6: Redirections (cmd < input.txt > output.txt) (CURRENT)
 * Future phases will add background jobs and job control.
 */

#include "picobox.h"
#include "cmd_spec.h"
#include "exec_helpers.h"
#include "pipe_helpers.h"
#include "redirect_helpers.h"
#include "../bnfc_shell/Parser.h"
#include "../bnfc_shell/Absyn.h"
#include "../bnfc_shell/Printer.h"
#include "../bnfc_shell/Skeleton.h"
#include <string.h>
#include <sys/wait.h>

/* External declarations for refactored commands */
extern void register_echo_command(void);
extern void register_pwd_command(void);
extern void register_true_command(void);
extern void register_false_command(void);
extern void register_basename_command(void);
extern void register_dirname_command(void);
extern void register_sleep_command(void);
extern void register_env_command(void);
extern void register_cat_command(void);
extern void register_wc_command(void);
extern void register_head_command(void);
extern void register_tail_command(void);
extern void register_touch_command(void);
extern void register_mkdir_command(void);
extern void register_cp_command(void);
extern void register_mv_command(void);
extern void register_rm_command(void);
extern void register_ln_command(void);
extern void register_chmod_command(void);
extern void register_stat_command(void);
extern void register_df_command(void);
extern void register_du_command(void);
extern void register_grep_command(void);
extern void register_find_command(void);
extern void register_ls_command(void);
extern void register_pkg_command(void);
extern void register_ai_command(void);
#define MAX_LINE_LENGTH 1024
#define PROMPT "$ "

/* No longer need command table - using fork/exec instead */

/*
 * Built-in command: exit
 * Returns -1 to signal shell exit
 */
static int builtin_exit(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return -1;
}

/*
 * Built-in command: help
 */
static int builtin_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("PicoBox BNFC Shell v%s (AI-Powered)\n", PICOBOX_VERSION);
    printf("Interactive command-line interface (BNFC-powered)\n\n");

    printf("Built-in commands:\n");
    printf("  exit       - Exit the shell\n");
    printf("  help       - Show this help message\n");
    printf("  cd [DIR]   - Change directory\n\n");

    printf("External commands:\n");
    printf("  Any command in your PATH (e.g., ls, cat, echo, grep, etc.)\n");
    printf("  Commands are executed in separate processes via fork/exec\n\n");

    printf("AI Assistant (Two Ways):\n");
    printf("  @<query>         - New: Natural language to command suggestion\n");
    printf("                     Example: @show me all .c files\n");
    printf("                     Uses: mysh_llm.py with RAG + LLM\n");
    printf("  AI <question>    - Legacy: Direct AI chat (grammar-based)\n");
    printf("                     Example: AI how do I list files\n");
    printf("                     Uses: cmd_ai.c with OpenAI API\n\n");

    printf("Pipelines & Redirections:\n");
    printf("  cmd1 | cmd2      - Pipeline (stdout of cmd1 → stdin of cmd2)\n");
    printf("  cmd < file       - Input redirection\n");
    printf("  cmd > file       - Output redirection\n");
    printf("  cmd >> file      - Append output\n");
    printf("  cmd1 ; cmd2      - Command sequence\n\n");

    printf("For help on a specific command, use: <command> --help\n");
    return EXIT_OK;
}

/*
 * Built-in command: cd
 */
static int builtin_cd(int argc, char **argv)
{
    const char *dir;

    if (argc < 2) {
        /* No argument - change to home directory */
        dir = getenv("HOME");
        if (!dir) {
            fprintf(stderr, "cd: HOME not set\n");
            return EXIT_ERROR;
        }
    } else {
        dir = argv[1];
    }

    if (chdir(dir) != 0) {
        perror("cd");
        return EXIT_ERROR;
    }

    return EXIT_OK;
}

/*
 * Handle AI query (lines starting with @)
 *
 * This is called when user types: @show me all files
 *
 * Process:
 * 1. Call Python script: python3 mysh_llm.py "show me all files"
 * 2. Read suggestion from script output
 * 3. Display suggestion to user with formatting
 * 4. Get y/n confirmation
 * 5. If yes: parse and execute suggestion via BNFC
 */
static void handle_llm_query(const char *query, ExecContext *ctx)
{
    char cmd[2048];
    char suggestion[1024];
    FILE *fp;
    char answer;
    int c;

    /* Validate input */
    if (!query || query[0] == '\0') {
        fprintf(stderr, "Error: Empty query\n");
        return;
    }

    /* Build command to call Python helper */
    const char *llm_script = getenv("MYSH_LLM_SCRIPT");
    if (!llm_script) {
        llm_script = "python3 mysh_llm.py";
    }

    /* Build command - basic escaping (assumes query doesn't contain quotes) */
    snprintf(cmd, sizeof(cmd), "%s \"%s\" 2>&1", llm_script, query);

    /* Call Python helper and capture output */
    fp = popen(cmd, "r");
    if (!fp) {
        perror("popen");
        fprintf(stderr, "Error: Failed to run AI helper script.\n");
        fprintf(stderr, "Make sure mysh_llm.py is in your current directory.\n");
        return;
    }

    /* Read suggestion from Python script */
    if (fgets(suggestion, sizeof(suggestion), fp) == NULL) {
        fprintf(stderr, "Error: AI helper returned no suggestion.\n");
        pclose(fp);
        return;
    }

    /* Remove trailing newline from suggestion */
    suggestion[strcspn(suggestion, "\n")] = '\0';

    /* Close pipe and check exit status */
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "Warning: AI helper exited with status %d\n", status);
    }

    /* Skip if suggestion is empty */
    if (strlen(suggestion) == 0) {
        fprintf(stderr, "Error: AI helper returned empty suggestion.\n");
        return;
    }

    /* Display suggestion to user with nice formatting */
    printf("\n");
    printf("💡 AI Suggested Command:\n");
    printf("   \033[1;32m%s\033[0m\n", suggestion);
    printf("\n");
    printf("Run this command? (y/n): ");
    fflush(stdout);

    /* Get user confirmation */
    if (scanf(" %c", &answer) != 1) {
        fprintf(stderr, "Error: Failed to read answer\n");
        /* Clear input buffer */
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }

    /* Clear rest of input line */
    while ((c = getchar()) != '\n' && c != EOF);

    /* Check if user approved */
    if (answer != 'y' && answer != 'Y') {
        printf("Command cancelled.\n");
        return;
    }

    /* User approved - parse and execute suggestion */
    printf("\n");

    /* Parse suggestion using BNFC parser */
    Input ast = psInput(suggestion);

    if (ast == NULL) {
        fprintf(stderr, "Parse error: AI suggestion has invalid syntax\n");
        fprintf(stderr, "Suggestion was: %s\n", suggestion);
        fprintf(stderr, "This might be a bug in the AI helper.\n");
        return;
    }

    /* Execute using visitor pattern (same as normal commands) */
    visitInput(ast, ctx);

    /* Check for errors */
    if (ctx->has_error) {
        fprintf(stderr, "Error: Command execution failed\n");
    }

    /* Check if command was 'exit' */
    if (ctx->should_exit) {
        printf("Note: AI suggested 'exit' - not executing for safety\n");
        ctx->should_exit = 0;  /* Don't actually exit */
    }

    /* Cleanup */
    free_Input(ast);
}

/*
 * Execute built-in command
 * Note: is_builtin() is now in exec_helpers.c/h
 */
static int exec_builtin(int argc, char **argv)
{
    if (strcmp(argv[0], "cd") == 0) {
        return builtin_cd(argc, argv);
    } else if (strcmp(argv[0], "exit") == 0) {
        return builtin_exit(argc, argv);
    } else if (strcmp(argv[0], "help") == 0) {
        return builtin_help(argc, argv);
    }

    fprintf(stderr, "Unknown builtin: %s\n", argv[0]);
    return EXIT_ERROR;
}

/*
 * Convert BNFC ListWord to argv array
 * Returns the number of words in the list
 */
static int count_words(ListWord list)
{
    int count = 0;
    while (list && list->word_) {
        count++;
        list = list->listword_;
    }
    return count;
}

/*
 * Convert BNFC Word + ListWord to argv array
 * Caller must free the returned array and all strings
 */
static char **words_to_argv(Word cmd_word, ListWord arg_words, int *argc_out)
{
    /* Count total words: command + arguments */
    int arg_count = count_words(arg_words);
    int total = 1 + arg_count;

    /* Allocate argv array (+ 1 for NULL terminator) */
    char **argv = malloc((total + 1) * sizeof(char *));
    if (!argv) {
        perror("malloc");
        return NULL;
    }

    /* First element is command name */
    argv[0] = strdup(cmd_word);
    if (!argv[0]) {
        free(argv);
        return NULL;
    }

    /* Fill in arguments */
    int i = 1;
    ListWord curr = arg_words;
    while (curr && curr->word_) {
        argv[i] = strdup(curr->word_);
        if (!argv[i]) {
            /* Cleanup on error */
            for (int j = 0; j < i; j++) {
                free(argv[j]);
            }
            free(argv);
            return NULL;
        }
        i++;
        curr = curr->listword_;
    }

    /* NULL terminator */
    argv[total] = NULL;

    *argc_out = total;
    return argv;
}

/*
 * Free argv array created by words_to_argv
 */
static void free_argv(char **argv)
{
    if (!argv) return;

    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

/*
 * Convert SimpleCommand to argv array (Phase 5 - new AST structure)
 * Caller must free the returned array and all strings
 */
static char **simple_command_to_argv(SimpleCommand sc, int *argc_out)
{
    if (!sc || sc->kind != is_Cmd) {
        fprintf(stderr, "Error: Invalid SimpleCommand\n");
        return NULL;
    }

    return words_to_argv(sc->u.cmd_.word_, sc->u.cmd_.listword_, argc_out);
}

/*
 * Count redirections in a ListRedirection
 * i belive this just traverses a
 */
static int count_redirections(ListRedirection list)
{
    int count = 0;
    while (list && list->redirection_) {
        count++;
        list = list->listredirection_;
    }
    return count;
}

/*
 * Extract redirections from SimpleCommand (Phase 6)
 * Caller must free the returned array (but NOT the filenames - they're from AST)
 */
static struct redirection *extract_redirections(SimpleCommand sc, int *count_out)
{
    struct redirection *redirs;
    int count;
    int i;
    ListRedirection curr;

    if (!sc || sc->kind != is_Cmd) {
        *count_out = 0;
        return NULL;
    }

    /* Count redirections */
    count = count_redirections(sc->u.cmd_.listredirection_);
    *count_out = count;

    if (count == 0) {
        return NULL;
    }

    /* Allocate array */
    redirs = malloc(count * sizeof(struct redirection));
    if (!redirs) {
        perror("malloc");
        *count_out = 0;
        return NULL;
    }

    /* Extract each redirection */
    i = 0;
    curr = sc->u.cmd_.listredirection_;
    while (curr && curr->redirection_) {
        Redirection r = curr->redirection_;

        switch (r->kind) {
            case is_RedirIn:
                redirs[i].type = REDIR_INPUT;
                redirs[i].filename = r->u.redirIn_.word_;
                break;

            case is_RedirOut:
                redirs[i].type = REDIR_OUTPUT;
                redirs[i].filename = r->u.redirOut_.word_;
                break;

            case is_RedirAppend:
                redirs[i].type = REDIR_APPEND;
                redirs[i].filename = r->u.redirAppend_.word_;
                break;

            default:
                fprintf(stderr, "Error: Unknown redirection type\n");
                free(redirs);
                *count_out = 0;
                return NULL;
        }

        i++;
        curr = curr->listredirection_;
    }

    return redirs;
}

/*
 * ====================================================================================
 * OLD MANUAL TRAVERSAL FUNCTIONS - KEPT FOR REFERENCE
 * These are replaced by the visitor pattern (see Skeleton.c)
 * ====================================================================================
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/*
 * Execute a simple command (Phase 6 - with redirections)
 *
 * This implements PROPER process isolation:
 * - Built-ins run in parent process (cd, exit, help)
 * - External commands run in child process via fork/exec
 * - Redirections are applied in child process before exec
 */
static int execute_single_simple_command(SimpleCommand sc)
{
    char **argv;
    int argc;
    int status;
    struct redirection *redirs;
    int redir_count;

    /* Convert SimpleCommand to argv */
    argv = simple_command_to_argv(sc, &argc);
    if (!argv) {
        fprintf(stderr, "Error: Failed to build argv\n");
        return EXIT_ERROR;
    }

    /* Extract redirections */
    redirs = extract_redirections(sc, &redir_count);

    /* Check if builtin - must run in parent process */
    if (is_builtin(argv[0])) {
        /* Note: Built-ins don't support redirections (for simplicity) */
        if (redir_count > 0) {
            fprintf(stderr, "Warning: Redirections not supported for built-in commands\n");
        }
        status = exec_builtin(argc, argv);
        free_argv(argv);
        if (redirs) free(redirs);
        return status;
    }

    /* Execute external command with redirections */
    status = exec_command_with_redirects(argv, redirs, redir_count);

    free_argv(argv);
    if (redirs) free(redirs);
    return status;
}

/*
 * Execute a pipeline (Phase 5 - cmd1 | cmd2 | cmd3)
 *
 * High-level job of this function:
 *  - Take the BNFC AST for a Pipeline
 *  - For each SimpleCommand in the pipeline, build an argv[] array
 *  - Collect all those argv[] arrays into argv_list
 *  - Call exec_pipeline(argv_list, count) to actually:
 *      - fork()
 *      - set up pipes
 *      - execvp() each command
 *  - Then free all temporary memory
 *
 * NOTE: This function does NOT create pipes or fork processes.
 *       That happens inside exec_pipeline().
 */
static int execute_pipeline_command(Pipeline pipeline)
{
    char ***argv_list;           // Will point to an array of (char **),
                                 // i.e., argv_list[i] is argv for command i
    int count;                   // Number of commands in the pipeline
    int i;
    int status;
    ListSimpleCommand curr;      // Iterator over the linked list of commands

    /* 1. Basic sanity check: make sure we got a real pipeline node */
    if (!pipeline || pipeline->kind != is_PipeLine) {
        fprintf(stderr, "Error: Invalid pipeline\n");
        return EXIT_ERROR;
    }

    /* 2. Count how many SimpleCommands are in the pipeline AST
     *
     *   Pipeline looks like:
     *       cmd1 | cmd2 | cmd3
     *
     *   In BNFC, that's usually represented as a linked list:
     *       listsimplecommand_ -> [cmd1] -> [cmd2] -> [cmd3] -> NULL
     */
    count = 0;
    curr = pipeline->u.pipeLine_.listsimplecommand_;
    while (curr && curr->simplecommand_) {
        count++;
        curr = curr->listsimplecommand_;   // move to next command in list
    }

    /* If for some reason there are no commands, just succeed and do nothing */
    if (count == 0) {
        return EXIT_OK;
    }

    /* 3. Allocate an array of char** with 'count' entries
     *
     *    argv_list[0] -> argv for first command
     *    argv_list[1] -> argv for second command
     *    ...
     *    argv_list[count-1] -> argv for last command
     *
     *    Each argv_list[i] will be something like:
     *        ["ls", "-l", NULL]
     *        ["grep", "error", NULL]
     *        ["wc", "-l", NULL]
     */
    argv_list = malloc(count * sizeof(char **));
    if (!argv_list) {
        perror("malloc");
        return EXIT_ERROR;
    }

    /* 4. Convert each SimpleCommand AST node into a normal argv[] array
     *
     *   simple_command_to_argv() will:
     *      - extract the command name and args from the AST
     *      - build a NULL-terminated char* array
     *      - set argc (number of entries, excluding the NULL)
     *
     *   Example:
     *      simple_command_to_argv("ls -la") -> ["ls", "-la", NULL]
     */
    i = 0;
    curr = pipeline->u.pipeLine_.listsimplecommand_;
    while (curr && curr->simplecommand_) {
        int argc;   // not really used here, but simple_command_to_argv needs it

        argv_list[i] = simple_command_to_argv(curr->simplecommand_, &argc);
        if (!argv_list[i]) {
            /* If conversion fails for some command, clean up everything so far */
            for (int j = 0; j < i; j++) {
                free_argv(argv_list[j]);
            }
            free(argv_list);
            return EXIT_ERROR;
        }

        i++;
        curr = curr->listsimplecommand_;
    }

    /* 5. Actually execute the pipeline
     *
     *    exec_pipeline(argv_list, count) is where the "real magic" happens:
     *      - It will create (count - 1) pipes
     *      - It will fork() 'count' child processes
     *      - In each child, it will dup2() the right pipe ends to stdin/stdout
     *      - Then it will execvp() the correct argv_list[i]
     *      - The parent will usually close pipe fds and wait for children
     */
    status = exec_pipeline(argv_list, count);

    /* 6. Free all the argv arrays we created and the argv_list wrapper */
    for (i = 0; i < count; i++) {
        free_argv(argv_list[i]);  // frees each char* and the array
    }
    free(argv_list);              // frees the outer char*** array

    /* 7. Return whatever status exec_pipeline() reported
     *    (e.g. last command's exit status or some error code)
     */
    return status;
}

/*
 * Execute an AI command (Phase 7)
 * Converts word list to a single query string and calls cmd_ai
 */
static int execute_ai_command(ListWord words)
{
    char query[2048] = "";
    size_t len = 0;
    ListWord curr = words;

    /* Build query string from word list */
    while (curr && curr->word_) {
        if (len > 0 && len < sizeof(query) - 1) {
            query[len++] = ' ';  /* Add space between words */
        }

        size_t word_len = strlen(curr->word_);
        if (len + word_len < sizeof(query) - 1) {
            strcpy(query + len, curr->word_);
            len += word_len;
        }

        curr = curr->listword_;
    }

    query[len] = '\0';

    /* Call AI command implementation */
    char *argv[] = {"AI", query, NULL};
    return cmd_ai(2, argv);
}

/*
 * Execute a command (can be simple, pipeline, or AI)
 */
static int execute_command(Command cmd)
{
    if (!cmd) {
        return EXIT_ERROR;
    }

    switch (cmd->kind) {
        case is_SimpleCmd:
            return execute_single_simple_command(cmd->u.simpleCmd_.simplecommand_);

        case is_PipeCmd:
            return execute_pipeline_command(cmd->u.pipeCmd_.pipeline_);

        case is_AICmd:
            return execute_ai_command(cmd->u.aICmd_.listword_);

        default:
            fprintf(stderr, "Error: Unknown command type\n");
            return EXIT_ERROR;
    }
}

/*
 * Execute parsed input (list of commands)
 * Commands can be simple (echo test) or pipelines (cat file | grep test)
 */
static int execute_input(Input input)
{
    int last_status = EXIT_OK;

    if (!input || input->kind != is_StartInput) {
        return EXIT_ERROR;
    }

    /* Execute each command in the list */
    ListCommand cmd_list = input->u.startInput_.listcommand_;

    while (cmd_list && cmd_list->command_) {
        int status = execute_command(cmd_list->command_);

        /* Check for exit signal */
        if (status == -1) {
            return -1;
        }

        last_status = status;
        cmd_list = cmd_list->listcommand_;
    }

    return last_status;
}

#pragma GCC diagnostic pop

/*
 * ====================================================================================
 * NEW VISITOR PATTERN IMPLEMENTATION
 * ====================================================================================
 */

/*
 * Initialize shell - register all refactored commands
 */
static void init_shell_commands(void)
{
    /* Register refactored commands using the new infrastructure */
    register_echo_command();
    register_pwd_command();
    register_true_command();
    register_false_command();
    register_basename_command();
    register_dirname_command();
    register_sleep_command();
    register_env_command();
    register_cat_command();
    register_wc_command();
    register_head_command();
    register_tail_command();
    register_touch_command();
    register_mkdir_command();
    register_cp_command();
    register_mv_command();
    register_rm_command();
    register_ln_command();
    register_chmod_command();
    register_stat_command();
    register_df_command();
    register_du_command();
    register_grep_command();
    register_find_command();
    register_ls_command();
    register_pkg_command();
    register_ai_command();
    /* All 27 commands including package manager and AI! */
}

/*
 * Main BNFC shell loop - VISITOR PATTERN VERSION
 * This version uses the visitor pattern for AST traversal
 */
int shell_bnfc_main_visitor(void)
{
    char line[MAX_LINE_LENGTH];
    Input ast;
    ExecContext *ctx;

    /* Initialize command registry */
    init_shell_commands();

    /* Create execution context */
    ctx = exec_context_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create execution context\n");
        return EXIT_ERROR;
    }

    printf("PicoBox BNFC Shell v%s (Visitor Pattern + Registry + AI)\n", PICOBOX_VERSION);
    printf("Type 'help' for available commands, 'exit' to quit.\n");
    printf("Features: 26+ commands, pipelines, redirections, dual AI systems\n");
    printf("\n");
    printf("💡 Try the AI assistant:\n");
    printf("   @show me all files        (New: mysh_llm.py with RAG)\n");
    printf("   AI how do I list files    (Legacy: cmd_ai.c)\n");
    printf("\n");

    while (1) {
        /* Print prompt */
        printf("%s", PROMPT);
        fflush(stdout);

        /* Read line */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            /* EOF (Ctrl+D) */
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

        /* Check for AI query (@ prefix) - BEFORE BNFC parsing
         *
         * If line starts with @, it's an AI query.
         * We handle this BEFORE calling psInput() so the grammar
         * never sees the @ character.
         *
         * This keeps our existing AICmd grammar rule completely
         * independent - "AI how do I..." still works via BNFC.
         *
         * Examples:
         *   @show all files      → AI helper
         *   AI how do I list     → BNFC parser → AICmd
         *   ls -la               → BNFC parser → SimpleCmd
         */
        if (line[0] == '@') {
            /* Skip the @ and pass rest to AI helper */
            handle_llm_query(line + 1, ctx);
            continue;  /* Go back to prompt, don't parse with BNFC */
        }

        /* Parse the line using BNFC parser */
        ast = psInput(line);

        if (ast == NULL) {
            fprintf(stderr, "Parse error: invalid syntax\n");
            continue;
        }

        /* Execute using visitor pattern - ONE CALL! */
        /* The visitor automatically traverses the entire AST */
        visitInput(ast, ctx);

        /* Free AST */
        free_Input(ast);

        /* Check for exit signal */
        if (ctx->should_exit) {
            break;
        }
    }

    exec_context_free(ctx);
    return EXIT_OK;
}

/*
 * Main BNFC shell loop - ORIGINAL VERSION (for comparison)
 */
int shell_bnfc_main(void)
{
    /* Use visitor pattern version */
    return shell_bnfc_main_visitor();

    /* Original manual traversal code below (commented out for reference)
    char line[MAX_LINE_LENGTH];
    Input ast;

    printf("PicoBox BNFC Shell v%s (Phase 7: AI Assistant)\n", PICOBOX_VERSION);
    printf("Type 'help' for available commands, 'exit' to quit.\n");
    printf("Features: fork/exec, pipes, redirections (<, >, >>), AI commands\n");
    printf("Try: AI how do I list all files in current directory\n\n");

    while (1) {
        printf("%s", PROMPT);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (line[0] == '\0') {
            continue;
        }

        ast = psInput(line);

        if (ast == NULL) {
            fprintf(stderr, "Parse error: invalid syntax\n");
            continue;
        }

        int status = execute_input(ast);

        free_Input(ast);

        if (status == -1) {
            break;
        }
    }

    return EXIT_OK;
    */
}
