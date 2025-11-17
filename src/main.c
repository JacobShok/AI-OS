#include "picobox.h"
#include "utils.h"
#include <string.h>
#include <libgen.h>

/*
 * Command dispatch table
 * Maps command names to their implementation functions, simple by assigning the adress of
 * a function to the function ptr cmd_func_t cmd_func using the main c params all functions follow
 * same format so this is possible
 */
const struct command commands[] = {
    /* Day 2-3: First 5 commands */
    {"echo", cmd_echo},
    {"pwd", cmd_pwd},
    {"cat", cmd_cat},
    {"mkdir", cmd_mkdir},
    {"touch", cmd_touch},

    /* Day 4-5: File operations */
    {"ls", cmd_ls},
    {"cp", cmd_cp},
    {"rm", cmd_rm},
    {"mv", cmd_mv},

    /* Day 6-7: Text processing */
    {"head", cmd_head},
    {"tail", cmd_tail},
    {"wc", cmd_wc},
    {"ln", cmd_ln},

    /* Day 8-9: Search utilities */
    {"grep", cmd_grep},
    {"find", cmd_find},
    {"basename", cmd_basename},
    {"dirname", cmd_dirname},

    /* Day 10-11: File permissions & system info */
    {"chmod", cmd_chmod},
    {"stat", cmd_stat},
    {"du", cmd_du},
    {"df", cmd_df},

    /* Day 12-13: Process & environment */
    {"env", cmd_env},
    {"sleep", cmd_sleep},
    {"true", cmd_true},
    {"false", cmd_false},

    /* Sentinel - end of array */
    {NULL, NULL}
};

/*
 * All 25 commands are now implemented in separate cmd_*.c files!
 */

/*
 * Find command by name in the dispatch table
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
 * Print usage information for picobox
 */
static void print_usage(void)
{
    printf("PicoBox v%s - BusyBox-style Unix utilities\n\n", PICOBOX_VERSION);
    printf("Usage: picobox <command> [arguments...]\n");
    printf("   or: <command> [arguments...]  (when invoked via symlink)\n\n");
    printf("Available commands:\n");

    for (int i = 0; commands[i].name != NULL; i++) {
        printf("  %s\n", commands[i].name);
    }

    printf("\nFor help on a specific command, use: <command> --help\n");
}

/*
 * Main entry point
 * Determines which command to run based on argv[0]
 */
int main(int argc, char **argv)
{
    char *program_name;
    char *command_name;
    cmd_func_t cmd_func;

    /* Handle empty arguments */
    if (argc < 1 || argv[0] == NULL) {
        fprintf(stderr, "picobox: invalid invocation\n");
        return EXIT_ERROR;
    }

    /* Extract the program name from argv[0] (handle paths like /usr/bin/ls) */
    program_name = basename(argv[0]);

    /*
     * Determine which command to run:
     * 1. If called as "picobox", use argv[1] as command name
     * 2. If called as anything else (symlink), use the program name
     * 3. If called as "picobox" with no args, enter shell mode
     */
    if (strcmp(program_name, "picobox") == 0) {
        /* Called as "picobox <command>" */
        if (argc < 2) {
            /* No command specified - enter interactive shell mode */
            /* Check environment variable to select shell implementation */
            char *use_bnfc = getenv("PICOBOX_BNFC");
            if (use_bnfc && strcmp(use_bnfc, "1") == 0) {
                return shell_bnfc_main();  /* BNFC-powered shell */
            } else {
                return shell_main();        /* Original shell */
            }
        }

        command_name = argv[1];

        /* Handle --help for picobox itself */
        if (strcmp(command_name, "--help") == 0 || strcmp(command_name, "-h") == 0) {
            print_usage();
            return EXIT_OK;
        }

        /* Shift arguments so command sees itself as argv[0] */
        argc--;
        argv++;
    } else {
        /* Called via symlink (e.g., "ls", "cat", etc.) */
        command_name = program_name;
    }

    /* Find and execute the command */
    cmd_func = find_command(command_name);
    if (cmd_func == NULL) {
        fprintf(stderr, "picobox: unknown command '%s'\n", command_name);
        fprintf(stderr, "Try 'picobox --help' for a list of available commands.\n");
        return EXIT_ERROR;
    }

    /* Execute the command - NEVER use exit() inside commands, always return */
    return cmd_func(argc, argv);
}
