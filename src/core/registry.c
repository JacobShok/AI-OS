/*
 * registry.c - Command Registry Implementation
 *
 * This file implements a simple global registry for commands.
 * Commands register themselves at startup, and the shell can
 * look them up by name at runtime.
 *
 * Implementation notes:
 *   - Uses a static array (simple, no dynamic allocation)
 *   - Max 64 commands (more than enough for this project)
 *   - Linear search (fast enough for small number of commands)
 *   - Thread-unsafe (not needed for single-threaded shell)
 */

#include <stddef.h>
#include <string.h>
#include "cmd_spec.h"

/* Maximum number of commands that can be registered */
#define MAX_COMMANDS 64

/* Global command registry - static array */
static const cmd_spec_t *command_registry[MAX_COMMANDS]; // Array of pointers for each command
static size_t command_count = 0;

/**
 * Register a command in the global registry
 *
 * This function is called during shell initialization.
 * Each command calls register_command(&cmd_NAME_spec) to
 * add itself to the registry.
 *
 * @param spec Pointer to command specification (must remain valid)
 */
void register_command(const cmd_spec_t *spec)
{
    /* Validate input */
    if (spec == NULL) {
        return;
    }

    /* Check if registry is full */
    if (command_count >= MAX_COMMANDS) {
        fprintf(stderr, "Warning: Command registry full, cannot register '%s'\n",
                spec->name ? spec->name : "(null)");
        return;
    }

    /* Add to registry */
    command_registry[command_count++] = spec;
}

/**
 * Find a command by name
 *
 * Performs linear search through the registry.
 * This is O(n) but fast enough for small n (< 64 commands).
 *
 * @param name Command name to search for
 * @return Pointer to command spec, or NULL if not found
 */
const cmd_spec_t *find_command(const char *name)
{
    size_t i;
    const cmd_spec_t *spec;

    /* Validate input */
    if (name == NULL) {
        return NULL;
    }

    /* Linear search */
    for (i = 0; i < command_count; i++) {
        spec = command_registry[i];

        /* Skip NULL entries (shouldn't happen, but be defensive) */
        if (spec == NULL || spec->name == NULL) {
            continue;
        }

        /* Compare names */
        if (strcmp(spec->name, name) == 0) {
            return spec;
        }
    }

    /* Not found */
    return NULL;
}

/**
 * Iterate over all registered commands
 *
 * Calls the provided callback function for each registered command.
 * Useful for implementing the "help" command that lists all commands.
 *
 * @param callback Function to call for each command
 * @param userdata User-provided data passed to callback
 */
void for_each_command(void (*callback)(const cmd_spec_t *spec, void *userdata),
                      void *userdata)
{
    size_t i;

    /* Validate callback */
    if (callback == NULL) {
        return;
    }

    /* Call callback for each command */
    for (i = 0; i < command_count; i++) {
        if (command_registry[i] != NULL) {
            callback(command_registry[i], userdata);
        }
    }
}
