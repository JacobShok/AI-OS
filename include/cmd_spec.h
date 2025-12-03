#ifndef CMD_SPEC_H
#define CMD_SPEC_H

#include <stdio.h>

/**
 * cmd_spec.h - Command Specification Infrastructure
 *
 * This header defines the standard anatomy for all commands in PicoBox.
 * Each command follows a consistent pattern:
 *   1. Argument parsing with argtable3
 *   2. Run function (main implementation)
 *   3. Print usage function (help text)
 *   4. Command specification structure
 *   5. Registration function
 *
 * This design allows commands to be:
 *   - Built as standalone binaries
 *   - Registered as shell built-ins
 *   - Packaged and distributed separately
 */

/**
 * Command specification structure
 *
 * This structure describes a command and provides function pointers
 * to its implementation and usage information.
 */
typedef struct cmd_spec {
    const char *name;        /* Command name (e.g., "ls", "cat") */
    const char *summary;     /* One-line description for help listing */
    const char *long_help;   /* Detailed help text (can be NULL) */

    /* Function to execute the command */
    int (*run)(int argc, char **argv);

    /* Function to print usage/help information */
    void (*print_usage)(FILE *out);
} cmd_spec_t;

/**
 * Registry functions
 *
 * These functions manage the global command registry.
 * Commands register themselves at startup, and the shell
 * can look them up by name at runtime.
 */

/**
 * Register a command in the global registry
 *
 * @param spec Pointer to command specification (must remain valid)
 */
void register_command(const cmd_spec_t *spec);

/**
 * Find a command by name
 *
 * @param name Command name to search for
 * @return Pointer to command spec, or NULL if not found
 */
const cmd_spec_t *find_command(const char *name);

/**
 * Iterate over all registered commands
 *
 * @param callback Function to call for each command
 * @param userdata User-provided data passed to callback
 *
 * Example usage:
 *   void print_cmd(const cmd_spec_t *spec, void *data) {
 *       printf("%s - %s\n", spec->name, spec->summary);
 *   }
 *   for_each_command(print_cmd, NULL);
 */
void for_each_command(void (*callback)(const cmd_spec_t *spec, void *userdata),
                      void *userdata);

#endif /* CMD_SPEC_H */
