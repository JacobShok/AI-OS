#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include <stddef.h>

/*
 * var_table.h - Shell variable storage
 *
 * Simple hash table for storing shell variables (e.g., FOO=bar).
 * Thread-safe for concurrent reads, but writes must be synchronized externally.
 */

/* Variable entry in hash table (linked list for collision resolution) */
typedef struct var_entry {
    char *name;
    char *value;
    struct var_entry *next;
} var_entry_t;

/* Variable hash table */
typedef struct var_table {
    size_t size;
    size_t count;
    var_entry_t **buckets;
} var_table_t;

/* Create new variable table with given size */
var_table_t *var_table_create(size_t size);

/* Destroy variable table and free all entries */
void var_table_destroy(var_table_t *table);

/* Set variable value (creates or updates) */
int var_table_set(var_table_t *table, const char *name, const char *value);

/* Get variable value (returns NULL if not found) */
const char *var_table_get(var_table_t *table, const char *name);

/* Unset (remove) variable */
int var_table_unset(var_table_t *table, const char *name);

#endif /* VAR_TABLE_H */
