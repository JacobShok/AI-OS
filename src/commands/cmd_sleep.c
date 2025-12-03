/*
 * cmd_sleep.c - Delay for specified time (REFACTORED VERSION)
 *
 * This is the refactored version using argtable3 and following
 * the standard command anatomy for PicoBox.
 *
 * Usage: sleep NUMBER[SUFFIX]
 *   NUMBER - Duration (can be decimal)
 *   SUFFIX - Optional: s (seconds), m (minutes), h (hours), d (days)
 *
 * Options:
 *   -h, --help    Display help message
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "argtable3.h"
#include "cmd_spec.h"
#include "picobox.h"

/* Forward declarations */
int sleep_run(int argc, char **argv);
void sleep_print_usage(FILE *out);

/* ===== SECTION 1: ARGTABLE STRUCTURES ===== */

static struct arg_lit *sleep_help;
static struct arg_str *sleep_duration;
static struct arg_end *sleep_end;
static void *sleep_argtable[4];

/* ===== SECTION 2: ARGTABLE BUILDER ===== */

static void build_sleep_argtable(void)
{
    sleep_help = arg_lit0("h", "help", "display this help and exit");
    sleep_duration = arg_str1(NULL, NULL, "NUMBER[SUFFIX]", "duration to sleep");
    sleep_end = arg_end(20);

    sleep_argtable[0] = sleep_help;
    sleep_argtable[1] = sleep_duration;
    sleep_argtable[2] = sleep_end;
    sleep_argtable[3] = NULL;
}

/* ===== SECTION 3: RUN FUNCTION ===== */

int sleep_run(int argc, char **argv)
{
    int nerrors;
    const char *duration_str;
    char *endptr;
    double seconds;
    char suffix;

    build_sleep_argtable();
    nerrors = arg_parse(argc, argv, sleep_argtable);

    /* Handle --help */
    if (sleep_help->count > 0) {
        sleep_print_usage(stdout);
        arg_freetable(sleep_argtable, 3);
        return EXIT_OK;
    }

    /* Handle parsing errors */
    if (nerrors > 0) {
        arg_print_errors(stderr, sleep_end, "sleep");
        fprintf(stderr, "Try 'sleep --help' for more information.\n");
        arg_freetable(sleep_argtable, 3);
        return EXIT_ERROR;
    }

    /* ===== ACTUAL COMMAND LOGIC ===== */

    duration_str = sleep_duration->sval[0];

    /* Parse the numeric part */
    seconds = strtod(duration_str, &endptr);
    if (endptr == duration_str || seconds < 0) {
        fprintf(stderr, "sleep: invalid time interval '%s'\n", duration_str);
        arg_freetable(sleep_argtable, 3);
        return EXIT_ERROR;
    }

    /* Parse optional suffix */
    if (*endptr != '\0') {
        suffix = *endptr;

        /* Check that suffix is the last character */
        if (*(endptr + 1) != '\0') {
            fprintf(stderr, "sleep: invalid time interval '%s'\n", duration_str);
            arg_freetable(sleep_argtable, 3);
            return EXIT_ERROR;
        }

        switch (suffix) {
            case 's':
                /* seconds - no conversion needed */
                break;
            case 'm':
                seconds *= 60;  /* minutes to seconds */
                break;
            case 'h':
                seconds *= 3600;  /* hours to seconds */
                break;
            case 'd':
                seconds *= 86400;  /* days to seconds */
                break;
            default:
                fprintf(stderr, "sleep: invalid time suffix '%c'\n", suffix);
                arg_freetable(sleep_argtable, 3);
                return EXIT_ERROR;
        }
    }

    /* Perform the sleep */
    sleep((unsigned int)seconds);

    arg_freetable(sleep_argtable, 3);
    return EXIT_OK;
}

/* ===== SECTION 4: PRINT USAGE FUNCTION ===== */

void sleep_print_usage(FILE *out)
{
    build_sleep_argtable();

    fprintf(out, "Usage: sleep ");
    arg_print_syntax(out, sleep_argtable, "\n");
    fprintf(out, "Pause for NUMBER seconds. SUFFIX may be:\n");
    fprintf(out, "  s - seconds (default)\n");
    fprintf(out, "  m - minutes\n");
    fprintf(out, "  h - hours\n");
    fprintf(out, "  d - days\n\n");
    fprintf(out, "Options:\n");
    arg_print_glossary(out, sleep_argtable, "  %-25s %s\n");
    fprintf(out, "\n");
    fprintf(out, "Examples:\n");
    fprintf(out, "  sleep 10        Pause for 10 seconds\n");
    fprintf(out, "  sleep 1.5       Pause for 1.5 seconds\n");
    fprintf(out, "  sleep 2m        Pause for 2 minutes\n");
    fprintf(out, "  sleep 1h        Pause for 1 hour\n");

    arg_freetable(sleep_argtable, 3);
}

/* ===== SECTION 5: COMMAND SPECIFICATION ===== */

cmd_spec_t cmd_sleep_spec = {
    .name = "sleep",
    .summary = "delay for a specified amount of time",
    .long_help = "Pause for NUMBER seconds. SUFFIX may be 's' for seconds (default), "
                 "'m' for minutes, 'h' for hours, or 'd' for days.",
    .run = sleep_run,
    .print_usage = sleep_print_usage
};

/* ===== SECTION 6: REGISTRATION FUNCTION ===== */

void register_sleep_command(void)
{
    register_command(&cmd_sleep_spec);
}

/* ===== SECTION 7: STANDALONE MAIN ===== */

#ifndef BUILTIN_ONLY
int main(int argc, char **argv)
{
    return cmd_sleep_spec.run(argc, argv);
}
#endif
