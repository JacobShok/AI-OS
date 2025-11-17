/*
 * shell_bnfc.c - BNFC-powered shell implementation
 *
 * This shell uses BNFC-generated parser instead of manual parsing.
 * Currently implements simple command execution (in-process).
 * Future phases will add fork/exec, pipes, redirections, and background jobs.
 */

#include "picobox.h"
#include "../bnfc_shell/Parser.h"
#include "../bnfc_shell/Absyn.h"
#include "../bnfc_shell/Printer.h"
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define PROMPT "$ "

/* External command table from main.c */
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

    printf("PicoBox BNFC Shell v%s\n", PICOBOX_VERSION);
    printf("Interactive command-line interface (BNFC-powered)\n\n");
    printf("Built-in commands:\n");
    printf("  exit       - Exit the shell\n");
    printf("  help       - Show this help message\n");
    printf("  cd [DIR]   - Change directory\n\n");
    printf("Available utility commands:\n");

    for (int i = 0; commands[i].name != NULL; i++) {
        printf("  %s\n", commands[i].name);
    }

    printf("\nFor help on a specific command, use: <command> --help\n");
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
 * Check if command is a built-in
 */
static int is_builtin(const char *cmd)
{
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "help") == 0);
}

/*
 * Execute built-in command
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
 * Execute a simple command (Phase 3 - still in-process execution)
 *
 * NOTE: This currently uses in-process execution (calling functions directly).
 * Phase 4 will replace this with fork/exec for proper process isolation.
 */
static int execute_simple_command(Command cmd)
{
    char **argv;
    int argc;
    int status;
    cmd_func_t cmd_func;

    /* Check that it's a SimpleCmd */
    if (cmd->kind != is_SimpleCmd) {
        fprintf(stderr, "Error: Unknown command type\n");
        return EXIT_ERROR;
    }

    /* Convert BNFC structures to argv */
    argv = words_to_argv(cmd->u.simpleCmd_.word_,
                         cmd->u.simpleCmd_.listword_,
                         &argc);

    if (!argv) {
        fprintf(stderr, "Error: Failed to build argv\n");
        return EXIT_ERROR;
    }

    /* Check if builtin */
    if (is_builtin(argv[0])) {
        status = exec_builtin(argc, argv);
        free_argv(argv);
        return status;
    }

    /* Find and execute regular command (IN-PROCESS for now) */
    cmd_func = find_command(argv[0]);
    if (cmd_func == NULL) {
        fprintf(stderr, "shell: command not found: %s\n", argv[0]);
        free_argv(argv);
        return EXIT_ERROR;
    }

    /* Execute command function directly (TODO: Phase 4 will use fork/exec) */
    status = cmd_func(argc, argv);

    free_argv(argv);
    return status;
}

/*
 * Execute parsed input (list of commands)
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
        int status = execute_simple_command(cmd_list->command_);

        /* Check for exit signal */
        if (status == -1) {
            return -1;
        }

        last_status = status;
        cmd_list = cmd_list->listcommand_;
    }

    return last_status;
}

/*
 * Main BNFC shell loop
 */
int shell_bnfc_main(void)
{
    char line[MAX_LINE_LENGTH];
    Input ast;

    printf("PicoBox BNFC Shell v%s\n", PICOBOX_VERSION);
    printf("Type 'help' for available commands, 'exit' to quit.\n");
    printf("Note: Currently using in-process execution. Fork/exec coming in Phase 4.\n\n");

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

        /* Parse the line using BNFC parser */
        ast = psInput(line);

        if (ast == NULL) {
            fprintf(stderr, "Parse error: invalid syntax\n");
            continue;
        }

        /* Execute the parsed commands */
        int status = execute_input(ast);

        /* Free AST */
        free_Input(ast);

        /* Check for exit signal */
        if (status == -1) {
            break;
        }
    }

    return EXIT_OK;
}
