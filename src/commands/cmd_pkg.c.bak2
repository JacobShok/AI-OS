/*
 * cmd_pkg.c - Package manager for PicoBox
 *
 * Manages installation of packages to ~/.mysh/
 * Package format: .tar.gz with pkg.json metadata
 */

#include "picobox.h"
#include "cmd_spec.h"
#include <argtable3.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>

// === PACKAGE STRUCTURE ===

// Package metadata from pkg.json
typedef struct {
    char name[64];
    char version[32];
    char description[256];
    char **binaries;      // Array of binary names to symlink
    int binary_count;
} PkgInfo;


// Installed package record
typedef struct {
    char name[64];
    char version[32];
    char description[256];
    char install_date[32];
    char path[512];       // Full path to package directory
} InstalledPkg;

// === GLOBALS ===
static char mysh_home[512];     // ~/.mysh/
static char pkg_dir[512];       // ~/.mysh/packages/
static char bin_dir[512];       // ~/.mysh/bin/
static char pkgdb_path[512];    // ~/.mysh/pkgdb.json

// === HELPER FUNCTIONS ===

/* Initialize package manager paths */
// Flow: Sets up global paths used throughout the package manager
static int init_pkg_paths(void) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "pkg: HOME environment variable not set\n");
        return -1;
    }

    snprintf(mysh_home, sizeof(mysh_home), "%s/.mysh", home);
    snprintf(pkg_dir, sizeof(pkg_dir), "%s/packages", mysh_home);
    snprintf(bin_dir, sizeof(bin_dir), "%s/bin", mysh_home);
    snprintf(pkgdb_path, sizeof(pkgdb_path), "%s/pkgdb.json", mysh_home);

    return 0;
}

/* Create directory structure if it doesn't exist */
static int ensure_directories(void) {
    struct stat st;

    // Create ~/.mysh
    if (stat(mysh_home, &st) != 0) {
        if (mkdir(mysh_home, 0755) != 0) {
            perror(mysh_home);
            return -1;
        }
    }

    // Create ~/.mysh/packages
    if (stat(pkg_dir, &st) != 0) {
        if (mkdir(pkg_dir, 0755) != 0) {
            perror(pkg_dir);
            return -1;
        }
    }

    // Create ~/.mysh/bin
    if (stat(bin_dir, &st) != 0) {
        if (mkdir(bin_dir, 0755) != 0) {
            perror(bin_dir);
            return -1;
        }
    }

    // Create empty pkgdb.json if it doesn't exist
    if (stat(pkgdb_path, &st) != 0) {
        FILE *fp = fopen(pkgdb_path, "w");
        if (!fp) {
            perror(pkgdb_path);
            return -1;
        }
        fprintf(fp, "{\"installed\":[]}\n");
        fclose(fp);
    }

    return 0;
}

/* Simple JSON parser for pkg.json uses string searches instead of external libs
 * Looks for "name": "value" patterns
 */
static int parse_pkg_json(const char *path, PkgInfo *info) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return -1;
    }

    char line[512];
    info->binary_count = 0;
    info->binaries = NULL;

    while (fgets(line, sizeof(line), fp)) {
        // Parse "name": "value"
        char *key = strstr(line, "\"name\"");
        if (key && !strstr(line, "binaries")) {
            char *val_start = strchr(key + 6, '"');
            if (val_start) {
                val_start++;  // Skip opening quote
                char *val_end = strchr(val_start, '"');
                if (val_end) {
                    size_t len = val_end - val_start;
                    if (len < sizeof(info->name)) {
                        strncpy(info->name, val_start, len);
                        info->name[len] = '\0';
                    }
                }
            }
        }

        // Parse "version"
        key = strstr(line, "\"version\"");
        if (key) {
            char *val_start = strchr(key + 9, '"');
            if (val_start) {
                val_start++;
                char *val_end = strchr(val_start, '"');
                if (val_end) {
                    size_t len = val_end - val_start;
                    if (len < sizeof(info->version)) {
                        strncpy(info->version, val_start, len);
                        info->version[len] = '\0';
                    }
                }
            }
        }

        // Parse "description"
        key = strstr(line, "\"description\"");
        if (key) {
            char *val_start = strchr(key + 13, '"');
            if (val_start) {
                val_start++;
                char *val_end = strchr(val_start, '"');
                if (val_end) {
                    size_t len = val_end - val_start;
                    if (len < sizeof(info->description)) {
                        strncpy(info->description, val_start, len);
                        info->description[len] = '\0';
                    }
                }
            }
        }

        // Parse "binaries" array - simple approach, expects ["bin1", "bin2"]
        key = strstr(line, "\"binaries\"");
        if (key) {
            char *array_start = strchr(key, '[');
            if (array_start) {
                // Count binaries
                char *p = array_start;
                int count = 0;
                while ((p = strchr(p, '"')) != NULL && p < strchr(array_start, ']')) {
                    count++;
                    p++;
                }

                info->binary_count = count / 2;  // Each binary has 2 quotes
                if (info->binary_count > 0) {
                    info->binaries = malloc(info->binary_count * sizeof(char *));

                    // Extract binary names
                    p = array_start;
                    for (int i = 0; i < info->binary_count; i++) {
                        p = strchr(p, '"');
                        if (p) {
                            p++;  // Skip opening quote
                            char *end = strchr(p, '"');
                            if (end) {
                                size_t len = end - p;
                                info->binaries[i] = malloc(len + 1);
                                strncpy(info->binaries[i], p, len);
                                info->binaries[i][len] = '\0';
                                p = end + 1;
                            }
                        }
                    }
                }
            }
        }
    }

    fclose(fp);

    // Validate required fields
    if (info->name[0] == '\0' || info->version[0] == '\0') {
        fprintf(stderr, "pkg: Invalid pkg.json - missing name or version\n");
        return -1;
    }

    return 0;
}

/* Free PkgInfo resources */
static void free_pkg_info(PkgInfo *info) {
    if (info->binaries) {
        for (int i = 0; i < info->binary_count; i++) {
            free(info->binaries[i]);
        }
        free(info->binaries);
        info->binaries = NULL;
    }
}

/* Extract tar.gz file to destination directory */
static int extract_tar(const char *tarfile, const char *dest_dir) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        // Child: execute tar
        execlp("tar", "tar", "-xzf", tarfile, "-C", dest_dir, NULL);
        perror("tar");
        _exit(127);
    }

    // Parent: wait for tar
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    }

    fprintf(stderr, "pkg: tar extraction failed\n");
    return -1;
}

/* Add package to pkgdb.json */
static int add_to_pkgdb(const PkgInfo *info, const char *install_path) {
    // Read existing pkgdb
    FILE *fp = fopen(pkgdb_path, "r");
    if (!fp) {
        perror(pkgdb_path);
        return -1;
    }

    // For simplicity, we'll append to the file
    // A full implementation would parse JSON properly
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = malloc(size + 1);
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    // Get current date
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[32];
    strftime(date, sizeof(date), "%Y-%m-%d", t);

    // Create new entry
    char entry[1024];
    snprintf(entry, sizeof(entry),
        "{\"name\":\"%s\",\"version\":\"%s\",\"description\":\"%s\",\"date\":\"%s\",\"path\":\"%s\"}",
        info->name, info->version, info->description, date, install_path);

    // Check if this is the first package
    char *insert_pos = strstr(content, "\"installed\":[");
    if (!insert_pos) {
        free(content);
        fprintf(stderr, "pkg: Invalid pkgdb.json format\n");
        return -1;
    }

    insert_pos += 13;  // Move past "installed":["

    // Write new pkgdb
    fp = fopen(pkgdb_path, "w");
    if (!fp) {
        free(content);
        perror(pkgdb_path);
        return -1;
    }

    // Write everything up to the array
    fwrite(content, 1, insert_pos - content, fp);

    // Add entry
    if (*insert_pos == ']') {
        // First package
        fprintf(fp, "%s]}", entry);
    } else {
        // Additional package
        fprintf(fp, "%s,", entry);
        fputs(insert_pos, fp);
    }

    fclose(fp);
    free(content);

    return 0;
}

/* Read installed packages from pkgdb.json */
static int read_pkgdb(InstalledPkg **packages, int *count) {
    FILE *fp = fopen(pkgdb_path, "r");
    if (!fp) {
        *count = 0;
        *packages = NULL;
        return 0;  // Empty is OK
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = malloc(size + 1);
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);

    // Count packages (count occurrences of "name":")
    int pkg_count = 0;
    char *p = content;
    while ((p = strstr(p, "\"name\":")) != NULL) {
        pkg_count++;
        p += 7;
    }

    if (pkg_count == 0) {
        free(content);
        *count = 0;
        *packages = NULL;
        return 0;
    }

    *packages = malloc(pkg_count * sizeof(InstalledPkg));
    *count = pkg_count;

    // Parse each package
    p = content;
    for (int i = 0; i < pkg_count; i++) {
        InstalledPkg *pkg = &(*packages)[i];
        memset(pkg, 0, sizeof(InstalledPkg));

        // Find name
        p = strstr(p, "\"name\":\"");
        if (p) {
            p += 8;
            char *end = strchr(p, '"');
            if (end) {
                size_t len = end - p;
                if (len < sizeof(pkg->name)) {
                    strncpy(pkg->name, p, len);
                    pkg->name[len] = '\0';
                }
            }
        }

        // Find version
        p = strstr(p, "\"version\":\"");
        if (p) {
            p += 11;
            char *end = strchr(p, '"');
            if (end) {
                size_t len = end - p;
                if (len < sizeof(pkg->version)) {
                    strncpy(pkg->version, p, len);
                    pkg->version[len] = '\0';
                }
            }
        }

        // Find description
        p = strstr(p, "\"description\":\"");
        if (p) {
            p += 15;
            char *end = strchr(p, '"');
            if (end) {
                size_t len = end - p;
                if (len < sizeof(pkg->description)) {
                    strncpy(pkg->description, p, len);
                    pkg->description[len] = '\0';
                }
            }
        }

        // Find install date
        p = strstr(p, "\"date\":\"");
        if (p) {
            p += 8;
            char *end = strchr(p, '"');
            if (end) {
                size_t len = end - p;
                if (len < sizeof(pkg->install_date)) {
                    strncpy(pkg->install_date, p, len);
                    pkg->install_date[len] = '\0';
                }
            }
        }

        // Find path
        p = strstr(p, "\"path\":\"");
        if (p) {
            p += 8;
            char *end = strchr(p, '"');
            if (end) {
                size_t len = end - p;
                if (len < sizeof(pkg->path)) {
                    strncpy(pkg->path, p, len);
                    pkg->path[len] = '\0';
                }
            }
        }
    }

    free(content);
    return 0;
}

/* Check if package is already installed */
static int is_installed(const char *name) {
    InstalledPkg *packages = NULL;
    int count = 0;

    if (read_pkgdb(&packages, &count) != 0) {
        return 0;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(packages[i].name, name) == 0) {
            free(packages);
            return 1;
        }
    }

    free(packages);
    return 0;
}

// === SUBCOMMAND IMPLEMENTATIONS ===

/* pkg install <package.tar.gz> */
static int pkg_install(const char *tarfile) {
    struct stat st;
    char temp_dir[512];
    char pkg_json_path[512];
    char install_path[512];
    PkgInfo info;
    int ret = EXIT_ERROR;

    // Validate tar file exists
    if (stat(tarfile, &st) != 0) {
        fprintf(stderr, "pkg install: %s: ", tarfile);
        perror("");
        return EXIT_ERROR;
    }

    // Create temp directory
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/pkg_install_%d", getpid());
    if (mkdir(temp_dir, 0755) != 0) {
        perror(temp_dir);
        return EXIT_ERROR;
    }

    // Extract to temp
    printf("Extracting package...\n");
    if (extract_tar(tarfile, temp_dir) != 0) {
        goto cleanup_temp;
    }

    // Read pkg.json
    snprintf(pkg_json_path, sizeof(pkg_json_path), "%s/pkg.json", temp_dir);
    if (parse_pkg_json(pkg_json_path, &info) != 0) {
        fprintf(stderr, "pkg install: Failed to parse pkg.json\n");
        goto cleanup_temp;
    }

    printf("Package: %s version %s\n", info.name, info.version);
    printf("Description: %s\n", info.description);

    // Check if already installed
    if (is_installed(info.name)) {
        fprintf(stderr, "pkg install: Package '%s' is already installed\n", info.name);
        fprintf(stderr, "             Use 'pkg remove %s' first to reinstall\n", info.name);
        free_pkg_info(&info);
        goto cleanup_temp;
    }

    // Create installation directory
    snprintf(install_path, sizeof(install_path), "%s/%s-%s",
             pkg_dir, info.name, info.version);

    if (mkdir(install_path, 0755) != 0) {
        perror(install_path);
        free_pkg_info(&info);
        goto cleanup_temp;
    }

    // Copy files from temp to install path
    printf("Installing to %s...\n", install_path);

    // Use cp command to copy recursively
    pid_t pid = fork();
    if (pid == 0) {
        execlp("cp", "cp", "-r", temp_dir, install_path, NULL);
        perror("cp");
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "pkg install: Failed to copy files\n");
        free_pkg_info(&info);
        goto cleanup_install;
    }

    // Create symlinks for binaries
    if (info.binary_count > 0) {
        printf("Creating symlinks for binaries:\n");
        for (int i = 0; i < info.binary_count; i++) {
            char target[512];
            char link_path[512];

            // Find the actual binary in the extracted package
            snprintf(target, sizeof(target), "%s/pkg_install_%d/%s",
                     install_path, getpid(), info.binaries[i]);
            snprintf(link_path, sizeof(link_path), "%s/%s",
                     bin_dir, info.binaries[i]);

            // Make binary executable
            chmod(target, 0755);

            // Create symlink
            unlink(link_path);  // Remove if exists
            if (symlink(target, link_path) != 0) {
                fprintf(stderr, "  Warning: Failed to create symlink for %s\n",
                        info.binaries[i]);
            } else {
                printf("  %s -> %s\n", info.binaries[i], target);
            }
        }
    }

    // Add to database
    if (add_to_pkgdb(&info, install_path) != 0) {
        fprintf(stderr, "pkg install: Failed to update package database\n");
        free_pkg_info(&info);
        goto cleanup_install;
    }

    printf("\nPackage '%s' installed successfully!\n", info.name);
    if (info.binary_count > 0) {
        printf("Binaries are available in %s/\n", bin_dir);
        printf("Make sure %s is in your PATH\n", bin_dir);
    }

    free_pkg_info(&info);
    ret = EXIT_OK;
    goto cleanup_temp;


// Key Design Decision 
// goto for cleanup: The install function uses goto cleanup_temp 
// for error handling. This ensures temp directory is always cleaned up:

cleanup_install:
    // Remove installation directory on failure
    pid = fork();
    if (pid == 0) {
        execlp("rm", "rm", "-rf", install_path, NULL);
        _exit(0);
    }
    waitpid(pid, NULL, 0);

cleanup_temp:
    // Remove temp directory
    pid = fork();
    if (pid == 0) {
        execlp("rm", "rm", "-rf", temp_dir, NULL);
        _exit(0);
    }
    waitpid(pid, NULL, 0);

    return ret;
}

/* pkg list */
static int pkg_list(void) {
    InstalledPkg *packages = NULL;
    int count = 0;

    if (read_pkgdb(&packages, &count) != 0) {
        return EXIT_ERROR;
    }

    if (count == 0) {
        printf("No packages installed.\n");
        return EXIT_OK;
    }

    printf("Installed packages:\n");
    printf("%-20s %-12s %s\n", "NAME", "VERSION", "DESCRIPTION");
    printf("%-20s %-12s %s\n", "----", "-------", "-----------");

    //print package info
    for (int i = 0; i < count; i++) {
        printf("%-20s %-12s %s\n",
               packages[i].name,
               packages[i].version,
               packages[i].description);
    }

    printf("\nTotal: %d package%s\n", count, count == 1 ? "" : "s");

    free(packages);
    return EXIT_OK;
}

/* pkg info <package-name> */
static int pkg_info(const char *name) {
    InstalledPkg *packages = NULL;
    int count = 0;

    if (read_pkgdb(&packages, &count) != 0) {
        return EXIT_ERROR;
    }

    for (int i = 0; i < count; i++) {
        // first print info
        if (strcmp(packages[i].name, name) == 0) {
            printf("Package: %s\n", packages[i].name);
            printf("Version: %s\n", packages[i].version);
            printf("Description: %s\n", packages[i].description);
            printf("Installed: %s\n", packages[i].install_date);
            printf("Location: %s\n", packages[i].path);

            // List files in package directory
            DIR *dir = opendir(packages[i].path);
            if (dir) {
                printf("\nFiles:\n");
                struct dirent *entry;
                // gets path
                while ((entry = readdir(dir)) != NULL) {
                    if (strcmp(entry->d_name, ".") != 0 &&
                        strcmp(entry->d_name, "..") != 0) {
                        printf("  %s\n", entry->d_name);
                    }
                }
                closedir(dir);
            }

            free(packages);
            return EXIT_OK;
        }
    }

    fprintf(stderr, "pkg info: Package '%s' is not installed\n", name);
    free(packages);
    return EXIT_ERROR;
}

/* pkg remove <package-name> */
static int pkg_remove(const char *name) {
    InstalledPkg *packages = NULL;
    int count = 0;
    char *found_path = NULL;

    if (read_pkgdb(&packages, &count) != 0) {
        return EXIT_ERROR;
    }

    // Find package
    for (int i = 0; i < count; i++) {
        if (strcmp(packages[i].name, name) == 0) {
            found_path = strdup(packages[i].path);
            break;
        }
    }

    if (!found_path) {
        fprintf(stderr, "pkg remove: Package '%s' is not installed\n", name);
        free(packages);
        return EXIT_ERROR;
    }

    printf("Removing package '%s'...\n", name);

    // Remove package directory
    pid_t pid = fork();
    if (pid == 0) {
        execlp("rm", "rm", "-rf", found_path, NULL); // removes found_path
        perror("rm");
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "pkg remove: Failed to remove package files\n");
        free(found_path);
        free(packages);
        return EXIT_ERROR;
    }

    // Remove symlinks from bin directory
    // For simplicity, we'll just warn that they should be removed manually
    printf("Note: Symlinks in %s may need to be removed manually\n", bin_dir);

    // Update pkgdb - remove this package
    // For simplicity, we'll rewrite the whole file without this package
    FILE *fp = fopen(pkgdb_path, "w");
    if (!fp) {
        perror(pkgdb_path);
        free(found_path);
        free(packages);
        return EXIT_ERROR;
    }

    fprintf(fp, "{\"installed\":[");
    int first = 1;
    for (int i = 0; i < count; i++) {
        if (strcmp(packages[i].name, name) != 0) {
            if (!first) fprintf(fp, ",");
            fprintf(fp, "{\"name\":\"%s\",\"version\":\"%s\",\"description\":\"%s\",\"date\":\"%s\",\"path\":\"%s\"}",
                    packages[i].name, packages[i].version, packages[i].description,
                    packages[i].install_date, packages[i].path);
            first = 0;
        }
    }
    fprintf(fp, "]}\n");
    fclose(fp);

    printf("Package '%s' removed successfully\n", name);

    free(found_path);
    free(packages);
    return EXIT_OK;
}

// === ARGTABLE STRUCTURES ===
static struct arg_lit *pkg_help;
static struct arg_str *pkg_subcommand;
static struct arg_str *pkg_args;
static struct arg_end *pkg_end;
static void *pkg_argtable[5];

// === ARGTABLE BUILDER ===
static void build_pkg_argtable(void) {
    pkg_help = arg_lit0("h", "help", "display this help and exit");
    pkg_subcommand = arg_str1(NULL, NULL, "COMMAND", "subcommand: install, list, remove, info");
    pkg_args = arg_strn(NULL, NULL, "ARG", 0, 10, "arguments for subcommand");
    pkg_end = arg_end(20);

    pkg_argtable[0] = pkg_help;
    pkg_argtable[1] = pkg_subcommand;
    pkg_argtable[2] = pkg_args;
    pkg_argtable[3] = pkg_end;
    pkg_argtable[4] = NULL;
}

// === PRINT USAGE ===
void pkg_print_usage(FILE *out) {
    build_pkg_argtable();

    fprintf(out, "Usage: pkg ");
    arg_print_syntax(out, pkg_argtable, "\n");
    fprintf(out, "\nPackage manager for PicoBox\n\n");

    fprintf(out, "Subcommands:\n");
    fprintf(out, "  install <file.tar.gz>   Install a package\n");
    fprintf(out, "  list                    List installed packages\n");
    fprintf(out, "  remove <name>           Remove an installed package\n");
    fprintf(out, "  info <name>             Show package information\n");
    fprintf(out, "\nOptions:\n");
    arg_print_glossary(out, pkg_argtable, "  %-25s %s\n");

    fprintf(out, "\nExample:\n");
    fprintf(out, "  pkg install hello-1.0.0.tar.gz\n");
    fprintf(out, "  pkg list\n");
    fprintf(out, "  pkg info hello\n");
    fprintf(out, "  pkg remove hello\n");

    arg_freetable(pkg_argtable, 4);
}

// === RUN FUNCTION ===
int pkg_run(int argc, char **argv) {
    int nerrors;
    int exit_code = EXIT_OK;

    // Initialize paths
    if (init_pkg_paths() != 0) {
        return EXIT_ERROR;
    }

    // Ensure directories exist
    if (ensure_directories() != 0) {
        return EXIT_ERROR;
    }

    // Build argtable
    build_pkg_argtable();

    // Parse arguments
    nerrors = arg_parse(argc, argv, pkg_argtable);

    // Handle --help
    if (pkg_help->count > 0) {
        pkg_print_usage(stdout);
        arg_freetable(pkg_argtable, 4);
        return EXIT_OK;
    }

    // Handle parsing errors
    if (nerrors > 0) {
        arg_print_errors(stderr, pkg_end, "pkg");
        pkg_print_usage(stderr);
        arg_freetable(pkg_argtable, 4);
        return EXIT_ERROR;
    }

    // Dispatch to subcommand
    const char *subcmd = pkg_subcommand->sval[0];

    if (strcmp(subcmd, "install") == 0) {
        if (pkg_args->count == 0) {
            fprintf(stderr, "pkg install: missing package file argument\n");
            pkg_print_usage(stderr);
            exit_code = EXIT_ERROR;
        } else {
            exit_code = pkg_install(pkg_args->sval[0]);
        }
    } else if (strcmp(subcmd, "list") == 0) {
        exit_code = pkg_list();
    } else if (strcmp(subcmd, "info") == 0) {
        if (pkg_args->count == 0) {
            fprintf(stderr, "pkg info: missing package name argument\n");
            pkg_print_usage(stderr);
            exit_code = EXIT_ERROR;
        } else {
            exit_code = pkg_info(pkg_args->sval[0]);
        }
    } else if (strcmp(subcmd, "remove") == 0 || strcmp(subcmd, "rm") == 0) {
        if (pkg_args->count == 0) {
            fprintf(stderr, "pkg remove: missing package name argument\n");
            pkg_print_usage(stderr);
            exit_code = EXIT_ERROR;
        } else {
            exit_code = pkg_remove(pkg_args->sval[0]);
        }
    } else {
        fprintf(stderr, "pkg: unknown subcommand '%s'\n", subcmd);
        pkg_print_usage(stderr);
        exit_code = EXIT_ERROR;
    }

    arg_freetable(pkg_argtable, 4);
    return exit_code;
}

// === COMMAND SPECIFICATION ===
cmd_spec_t cmd_pkg_spec = {
    .name = "pkg",
    .summary = "package manager for PicoBox",
    .long_help = "Install, list, remove, and query packages in ~/.mysh/",
    .run = pkg_run,
    .print_usage = pkg_print_usage
};

// === REGISTRATION ===
void register_pkg_command(void) {
    register_command(&cmd_pkg_spec);
}

// === STANDALONE MAIN ===
#ifndef BUILTIN_ONLY
int main(int argc, char **argv) {
    return cmd_pkg_spec.run(argc, argv);
}
#endif
