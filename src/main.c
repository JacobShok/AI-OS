#include "picobox.h"
#include "cmd_spec.h"
#include "utils.h"
#include <string.h>
#include <libgen.h>

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

/* Initialize command registry */
static void init_commands(void)
{
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
}

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

    /* Package manager */
    {"pkg", cmd_pkg},

    /* Sentinel - end of array */
    {NULL, NULL}
};

/*
 * All 25 commands are now implemented in separate cmd_*.c files!
 */

/*
 * Find command by name in the OLD dispatch table
 * NOTE: This is the legacy lookup. New refactored commands use the registry.
 */
static cmd_func_t find_legacy_command(const char *name)
{
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return commands[i].func;
        }
    }
    return NULL;
}

/*
 * Print all commands in JSON format for AI helper
 *
 * Output format:
 * {
 *   "commands": [
 *     {
 *       "name": "echo",
 *       "summary": "print text to stdout",
 *       "description": "Echo the STRING(s) to standard output...",
 *       "usage": "echo [OPTION]... [STRING]..."
 *     },
 *     ...
 *   ]
 * }
 */
static void print_commands_json(void)
{
    int first = 1;

    printf("{\n");
    printf("  \"commands\": [\n");

    /* Iterate through command dispatch table */
    for (int i = 0; commands[i].name != NULL; i++) {
        /* Add comma before all but first command */
        if (!first) {
            printf(",\n");
        }
        first = 0;

        printf("    {\n");
        printf("      \"name\": \"%s\",\n", commands[i].name);

        /* Get command spec from registry for detailed info */
        const cmd_spec_t *spec = find_command(commands[i].name);

        if (spec) {
            /* Use detailed info from spec */
            printf("      \"summary\": \"%s\",\n",
                   spec->summary ? spec->summary : "");

            /* Escape quotes and newlines in description */
            if (spec->long_help) {
                printf("      \"description\": \"");
                for (const char *p = spec->long_help; *p; p++) {
                    if (*p == '"' || *p == '\\') {
                        putchar('\\');
                    }
                    if (*p == '\n') {
                        printf("\\n");
                    } else {
                        putchar(*p);
                    }
                }
                printf("\",\n");
            } else {
                printf("      \"description\": \"\",\n");
            }

            /* Usage string */
            printf("      \"usage\": \"%s [OPTIONS]...\"\n", commands[i].name);
        } else {
            /* Fallback if no spec found */
            printf("      \"summary\": \"Unix utility\",\n");
            printf("      \"description\": \"See '%s --help' for details\",\n",
                   commands[i].name);
            printf("      \"usage\": \"%s [OPTIONS]...\"\n", commands[i].name);
        }

        printf("    }");
    }

    printf("\n  ]\n");
    printf("}\n");
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

    /* Initialize command registry */
    init_commands();

    /* Handle --commands-json flag for AI integration */
    if (argc >= 2 && strcmp(argv[1], "--commands-json") == 0) {
        print_commands_json();
        return EXIT_OK;
    }

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
            /* Use BNFC shell by default (has fork/exec pipelines) */
            return shell_bnfc_main();
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
    cmd_func = find_legacy_command(command_name);
    if (cmd_func == NULL) {
        fprintf(stderr, "picobox: unknown command '%s'\n", command_name);
        fprintf(stderr, "Try 'picobox --help' for a list of available commands.\n");
        return EXIT_ERROR;
    }

    /* Execute the command - NEVER use exit() inside commands, always return */
    return cmd_func(argc, argv);
}
