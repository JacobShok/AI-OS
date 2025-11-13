#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <libgen.h>

/* ===== Error Handling Functions ===== */

// error message to stderr stream
void error_msg(const char *msg)
{
    fprintf(stderr, "picobox: %s\n", msg);
}

int perror_return(const char *msg, int code)
{
    perror(msg);
    return code;
}

// function to teach users how to call command 
void usage(const char *cmd, const char *msg)
{
    fprintf(stderr, "Usage: %s %s\n", cmd, msg);
}

/* ===== String Utilities ===== */

// checks if a string ends with a specific suffix. It returns 1 (true) if it does, 0 (false) if it doesn't
int str_ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix) {
        return 0;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return 0;
    }

    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

// simply returns if a file has the same prefix
int str_starts_with(const char *str, const char *prefix)
{
    if (!str || !prefix) {
        return 0;
    }

    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// trims leading and ending whitespace
char *trim_whitespace(char *str)
{
    char *end;

    if (!str) {
        return str;
    }

    /* Trim leading whitespace */
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    /* Trim trailing whitespace */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    /* Write new null terminator */
    *(end + 1) = '\0';

    return str;
}


/* ===== Path Manipulation ===== */


char *path_join(const char *base, const char *name)
{
    if (!base || !name) {
        return NULL;
    }

    size_t base_len = strlen(base);
    size_t name_len = strlen(name);
    int need_slash = (base_len > 0 && base[base_len - 1] != '/') ? 1 : 0;

    size_t total_len = base_len + need_slash + name_len + 1;
    char *result = malloc(total_len);
    if (!result) {
        return NULL;
    }

    strcpy(result, base);
    if (need_slash) {
        strcat(result, "/");
    }
    strcat(result, name);

    return result;
}

char *get_basename(const char *path)
{
    if (!path) {
        return NULL;
    }

    /* Need to copy because basename() may modify its argument */
    char *path_copy = strdup(path);
    if (!path_copy) {
        return NULL;
    }

    char *base = basename(path_copy);
    char *result = strdup(base);

    free(path_copy);
    return result;
}

char *get_dirname(const char *path)
{
    if (!path) {
        return NULL;
    }

    /* Need to copy because dirname() may modify its argument */
    char *path_copy = strdup(path);
    if (!path_copy) {
        return NULL;
    }

    char *dir = dirname(path_copy);
    char *result = strdup(dir);

    free(path_copy);
    return result;
}


/* ===== File Utilities ===== */

// This is a simple utility function that checks if a given path is a directory

int is_directory(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0) {
        return 0;
    }

    return S_ISDIR(st.st_mode);
}

int is_regular_file(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0) {
        return 0;
    }

    return S_ISREG(st.st_mode);
}

int file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

// copy data loop is not correct
ssize_t copy_file(const char *src, const char *dest)
{
    int src_fd = -1;
    int dest_fd = -1;
    ssize_t total_bytes = 0;
    ssize_t bytes_read;
    char buffer[8192];

    /* Open source file */
    src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        return -1;
    }

    /* Open destination file */
    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        close(src_fd);
        return -1;
    }

    /* Copy data */
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            close(src_fd);
            close(dest_fd);
            return -1;
        }
        total_bytes += bytes_written;
    }

    if (bytes_read < 0) {
        close(src_fd);
        close(dest_fd);
        return -1;
    }

    close(src_fd);
    close(dest_fd);

    return total_bytes;
}


/* ===== Human-Readable Formatting ===== */

char *format_size(off_t size, char *buf, size_t bufsize)
{
    const char *units[] = {"B", "K", "M", "G", "T"};
    int unit_index = 0;
    double size_d = (double)size;

    while (size_d >= 1024.0 && unit_index < 4) {
        size_d /= 1024.0;
        unit_index++;
    }

    if (unit_index == 0) {
        snprintf(buf, bufsize, "%ld%s", (long)size, units[unit_index]);
    } else {
        snprintf(buf, bufsize, "%.1f%s", size_d, units[unit_index]);
    }

    return buf;
}

char *format_time(time_t t, char *buf, size_t bufsize)
{
    struct tm *tm_info = localtime(&t);
    if (!tm_info) {
        snprintf(buf, bufsize, "unknown");
        return buf;
    }

    /* Format: "Jan 15 14:30" for same year, "Jan 15  2023" for other years */
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);

    if (now_tm && tm_info->tm_year == now_tm->tm_year) {
        strftime(buf, bufsize, "%b %e %H:%M", tm_info);
    } else {
        strftime(buf, bufsize, "%b %e  %Y", tm_info);
    }

    return buf;
}
