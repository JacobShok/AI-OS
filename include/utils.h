#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <time.h>

/* ===== Error Handling Functions ===== */

/**
 * Print error message to stderr
 * @param msg Error message to print
 */
void error_msg(const char *msg);

/**
 * Print error with perror() and return error code
 * @param msg Message prefix for perror
 * @param code Return code to return
 * @return The error code passed in
 */
int perror_return(const char *msg, int code);

/**
 * Print usage/help message for a command
 * @param cmd Command name
 * @param msg Usage message
 */
void usage(const char *cmd, const char *msg);


/* ===== String Utilities ===== */

/**
 * Check if string ends with suffix
 * @param str String to check
 * @param suffix Suffix to look for
 * @return 1 if str ends with suffix, 0 otherwise
 */
int str_ends_with(const char *str, const char *suffix);

/**
 * Check if string starts with prefix
 * @param str String to check
 * @param prefix Prefix to look for
 * @return 1 if str starts with prefix, 0 otherwise
 */
int str_starts_with(const char *str, const char *prefix);

/**
 * Remove leading and trailing whitespace (modifies in place)
 * @param str String to trim
 * @return Pointer to trimmed string
 */
char *trim_whitespace(char *str);


/* ===== Path Manipulation ===== */

/**
 * Safely join two path components
 * @param base Base path
 * @param name Path component to append
 * @return Newly allocated string (caller must free), or NULL on error
 */
char *path_join(const char *base, const char *name);

/**
 * Extract filename from path (allocates new string)
 * @param path Full path
 * @return Newly allocated string with basename (caller must free)
 */
char *get_basename(const char *path);

/**
 * Extract directory from path (allocates new string)
 * @param path Full path
 * @return Newly allocated string with dirname (caller must free)
 */
char *get_dirname(const char *path);


/* ===== File Utilities ===== */

/**
 * Check if path is a directory
 * @param path Path to check
 * @return 1 if directory, 0 otherwise
 */
int is_directory(const char *path);

/**
 * Check if path is a regular file
 * @param path Path to check
 * @return 1 if regular file, 0 otherwise
 */
int is_regular_file(const char *path);

/**
 * Check if file exists
 * @param path Path to check
 * @return 1 if exists, 0 otherwise
 */
int file_exists(const char *path);

/**
 * Copy file from src to dest
 * @param src Source file path
 * @param dest Destination file path
 * @return Number of bytes copied, or -1 on error
 */
ssize_t copy_file(const char *src, const char *dest);


/* ===== Human-Readable Formatting ===== */

/**
 * Format file size in human-readable format (B, K, M, G, T)
 * @param size Size in bytes
 * @param buf Buffer to write formatted string
 * @param bufsize Size of buffer
 * @return Pointer to buf
 */
char *format_size(off_t size, char *buf, size_t bufsize);

/**
 * Format timestamp in readable format
 * @param t Timestamp
 * @param buf Buffer to write formatted string
 * @param bufsize Size of buffer
 * @return Pointer to buf
 */
char *format_time(time_t t, char *buf, size_t bufsize);

#endif /* UTILS_H */
