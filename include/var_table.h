#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include <stddef.h>

typedef struct var_entry {
    char *name;
    char *value;
    struct var_entry *next;
} var_entry_t;

typedef struct var_table {
    var_entry_t **buckets;
    size_t size;
    size_t count;
} var_table_t;

var_table_t *var_table_create(size_t size);
void var_table_destroy(var_table_t *table);
int var_table_set(var_table_t *table, const char *name, const char *value);
const char *var_table_get(var_table_t *table, const char *name);
int var_table_unset(var_table_t *table, const char *name);

#endif /* VAR_TABLE_H */
