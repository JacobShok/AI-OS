#include "picobox.h"
#include "utils.h"
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
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
 * Parse command line into arguments
 * Handles basic quoting and whitespace
 * Returns number of arguments parsed, or -1 on error
 */
static int parse_line(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *p = line;
    int in_quotes = 0;
    int in_word = 0;
    char *word_start = NULL;

    while (*p && argc < max_args - 1) {
        if (*p == '"') {
            if (!in_word) {
                /* Start of quoted word */
                in_quotes = 1;
                in_word = 1;
                word_start = p + 1;
            } else if (in_quotes) {
                /* End of quoted word */
                *p = '\0';
                argv[argc++] = word_start;
                in_quotes = 0;
                in_word = 0;
            }
            p++;
        } else if (isspace(*p)) {
            if (in_quotes) {
                /* Space inside quotes is part of the word */
                p++;
            } else if (in_word) {
                /* End of unquoted word */
                *p = '\0';
                argv[argc++] = word_start;
                in_word = 0;
                p++;
            } else {
                /* Skip whitespace */
                p++;
            }
        } else {
            if (!in_word) {
                /* Start of unquoted word */
                in_word = 1;
                word_start = p;
            }
            p++;
        }
    }

    /* Handle last word if we ended in the middle of one */
    if (in_word && !in_quotes) {
        argv[argc++] = word_start;
    } else if (in_quotes) {
        fprintf(stderr, "shell: unclosed quote\n");
        return -1;
    }

    argv[argc] = NULL;
    return argc;
}

/*
 * Built-in command: exit
 */
static int builtin_exit(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return -1;  /* Signal to exit shell */
}

/*
 * Built-in command: help
 */
static int builtin_help(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("PicoBox Shell v%s\n", PICOBOX_VERSION);
    printf("Interactive command-line interface\n\n");
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
 * Execute a command
 * Returns the command's exit code, or -1 to signal shell exit
 */
static int execute_command(int argc, char **argv)
{
    const char *cmd_name;
    cmd_func_t cmd_func;

    if (argc == 0) {
        return EXIT_OK;
    }

    cmd_name = argv[0];

    /* Check for built-in commands first */
    if (strcmp(cmd_name, "exit") == 0) {
        return builtin_exit(argc, argv);
    } else if (strcmp(cmd_name, "help") == 0) {
        return builtin_help(argc, argv);
    } else if (strcmp(cmd_name, "cd") == 0) {
        return builtin_cd(argc, argv);
    }

    /* Find and execute regular command */
    cmd_func = find_command(cmd_name);
    if (cmd_func == NULL) {
        fprintf(stderr, "shell: command not found: %s\n", cmd_name);
        return EXIT_ERROR;
    }

    return cmd_func(argc, argv);
}

/*
 * Main shell loop
 */
int shell_main(void)
{
    char line[MAX_LINE_LENGTH];
    char *argv[MAX_ARGS];
    int argc;
    int status;

    printf("PicoBox Shell v%s\n", PICOBOX_VERSION);
    printf("Type 'help' for available commands, 'exit' to quit.\n");

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

        /* Parse line into arguments */
        argc = parse_line(line, argv, MAX_ARGS);
        if (argc < 0) {
            continue;  /* Parse error already printed */
        }

        if (argc == 0) {
            continue;  /* Empty command */
        }

        /* Execute command */
        status = execute_command(argc, argv);
        if (status == -1) {
            /* Exit signal from built-in exit command */
            break;
        }
    }

    return EXIT_OK;
}
