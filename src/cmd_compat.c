/*
 * cmd_compat.c - Compatibility layer for refactored commands
 *
 * This file provides stub implementations for commands that have been
 * refactored to use the new argtable3/registry infrastructure.
 *
 * These stubs redirect to the new implementations via the registry.
 * This allows the old dispatch table in main.c to continue working
 * while we gradually refactor all commands.
 */

#include "picobox.h"
#include "cmd_spec.h"

/*
 * Stub for echo command
 */
int cmd_echo(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("echo");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "echo: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for pwd command
 */
int cmd_pwd(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("pwd");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "pwd: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for true command
 */
int cmd_true(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("true");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "true: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for false command
 */
int cmd_false(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("false");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "false: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for basename command
 */
int cmd_basename(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("basename");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "basename: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for dirname command
 */
int cmd_dirname(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("dirname");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "dirname: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for sleep command
 */
int cmd_sleep(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("sleep");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "sleep: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for env command
 */
int cmd_env(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("env");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "env: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for cat command
 */
int cmd_cat(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("cat");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "cat: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for wc command
 */
int cmd_wc(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("wc");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "wc: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for head command
 */
int cmd_head(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("head");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "head: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for tail command
 */
int cmd_tail(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("tail");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "tail: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for touch command
 */
int cmd_touch(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("touch");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "touch: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for mkdir command
 */
int cmd_mkdir(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("mkdir");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "mkdir: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for cp command
 */
int cmd_cp(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("cp");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "cp: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for mv command
 */
int cmd_mv(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("mv");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "mv: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for rm command
 */
int cmd_rm(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("rm");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "rm: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for ln command
 */
int cmd_ln(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("ln");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "ln: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for chmod command
 */
int cmd_chmod(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("chmod");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "chmod: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for stat command
 */
int cmd_stat(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("stat");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "stat: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for df command
 */
int cmd_df(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("df");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "df: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for du command
 */
int cmd_du(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("du");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "du: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for grep command
 */
int cmd_grep(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("grep");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "grep: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for find command
 */
int cmd_find(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("find");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "find: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for ls command
 */
int cmd_ls(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("ls");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "ls: command not found in registry\n");
    return EXIT_ERROR;
}

/*
 * Stub for pkg command
 */
int cmd_pkg(int argc, char **argv)
{
    const cmd_spec_t *spec = find_command("pkg");
    if (spec) {
        return spec->run(argc, argv);
    }
    fprintf(stderr, "pkg: command not found in registry\n");
    return EXIT_ERROR;
}
