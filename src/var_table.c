#include "var_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* djb2 hash function */
static unsigned long hash(const char *str)
{
    unsigned long h = 5381;
    int c;

    while ((c = *str++)) {
        h = ((h << 5) + h) + c;
    }

    return h;
}

var_table_t *var_table_create(size_t size)
{
    var_table_t *table = malloc(sizeof(var_table_t));
    if (!table) {
        perror("malloc");
        return NULL;
    }

    table->size = size;
    table->count = 0;
    table->buckets = calloc(size, sizeof(var_entry_t *));
    if (!table->buckets) {
        perror("calloc");
        free(table);
        return NULL;
    }

    return table;
}

void var_table_destroy(var_table_t *table)
{
    if (!table) {
        return;
    }

    for (size_t i = 0; i < table->size; i++) {
        var_entry_t *entry = table->buckets[i];
        while (entry) {
            var_entry_t *next = entry->next;
            free(entry->name);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }

    free(table->buckets);
    free(table);
}

int var_table_set(var_table_t *table, const char *name, const char *value)
{
    if (!table || !name || !value) {
        return -1;
    }

    unsigned long h = hash(name);
    size_t index = h % table->size;

    /* Check if variable already exists */
    var_entry_t *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            /* Update existing value */
            char *new_value = strdup(value);
            if (!new_value) {
                perror("strdup");
                return -1;
            }
            free(entry->value);
            entry->value = new_value;
            return 0;
        }
        entry = entry->next;
    }

    /* Create new entry */
    entry = malloc(sizeof(var_entry_t));
    if (!entry) {
        perror("malloc");
        return -1;
    }

    entry->name = strdup(name);
    entry->value = strdup(value);
    if (!entry->name || !entry->value) {
        perror("strdup");
        free(entry->name);
        free(entry->value);
        free(entry);
        return -1;
    }

    /* Insert at head of bucket */
    entry->next = table->buckets[index];
    table->buckets[index] = entry;
    table->count++;
    return 0;
}

const char *var_table_get(var_table_t *table, const char *name)
{
    if (!table || !name) {
        return NULL;
    }

    unsigned long h = hash(name);
    size_t index = h % table->size;

    var_entry_t *entry = table->buckets[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

int var_table_unset(var_table_t *table, const char *name)
{
    if (!table || !name) {
        return -1;
    }

    unsigned long h = hash(name);
    size_t index = h % table->size;

    var_entry_t **ptr = &table->buckets[index];
    while (*ptr) {
        var_entry_t *entry = *ptr;
        if (strcmp(entry->name, name) == 0) {
            *ptr = entry->next;
            free(entry->name);
            free(entry->value);
            free(entry);
            table->count--;
            return 0;
        }
        ptr = &entry->next;
    }

    return -1;
}
