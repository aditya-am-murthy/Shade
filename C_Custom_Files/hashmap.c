#include "hashmap.h"
#include <string.h>
#include <stdlib.h>

static size_t hash(const char* key, size_t capacity) {
    size_t hash_value = 5381;
    int c;
    while ((c = *key++)) {
        hash_value = ((hash_value << 5) + hash_value) + c; // hash * 33 + c
    }
    return hash_value % capacity;
}

HashMap* hashmap_create(size_t capacity) {
    HashMap* map = malloc(sizeof(HashMap));
    if (!map) return NULL;

    map->entries = calloc(capacity, sizeof(Entry*));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    map->capacity = capacity;
    map->size = 0;
    map->resize_threshold = (size_t)(capacity * 0.7);
    return map;
}

void hashmap_destroy(HashMap* map) {
    if (!map) return;
    for (size_t i = 0; i < map->capacity; ++i) {
        Entry* entry = map->entries[i];
        while (entry) {
            Entry* next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    free(map->entries);
    free(map);
}


bool hashmap_get(HashMap* map, const char* key, uint16_t* value1, uint16_t* value2) {
    size_t index = hash(key, map->capacity);
    Entry* entry = map->entries[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            *value1 = entry->value1;
            *value2 = entry->value2;
            return true;
        }
        entry = entry->next;
    }
    return false;
}

bool hashmap_delete(HashMap* map, const char* key) {
    size_t index = hash(key, map->capacity);
    Entry* entry = map->entries[index];
    Entry* prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                map->entries[index] = entry->next;
            }
            free(entry->key);
            free(entry);
            map->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    return false;
}

static void hashmap_resize(HashMap* map) {
    size_t new_capacity = map->capacity * 2;
    Entry** new_entries = calloc(new_capacity, sizeof(Entry*));
    if (!new_entries) return;

    // Rehash all entries into the new table
    for (size_t i = 0; i < map->capacity; i++) {
        Entry* entry = map->entries[i];
        while (entry) {
            Entry* next = entry->next;
            size_t new_index = hash(entry->key, new_capacity);
            entry->next = new_entries[new_index];
            new_entries[new_index] = entry;
            entry = next;
        }
    }

    free(map->entries);
    map->entries = new_entries;
    map->capacity = new_capacity;
    map->resize_threshold = (size_t)(new_capacity * 0.7);
}

bool hashmap_set(HashMap* map, const char* key, uint16_t value1, uint16_t value2) {
    size_t index = hash(key, map->capacity);
    Entry* entry = map->entries[index];

    // Check if the key already exists and update
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value1 = value1 & 0x0FFF; // Ensure 12-bit
            entry->value2 = value2 & 0x0FFF;
            return true;
        }
        entry = entry->next;
    }

    // Create new entry
    Entry* new_entry = malloc(sizeof(Entry));
    if (!new_entry) return false;

    new_entry->key = malloc(strlen(key) + 1);
    if (!new_entry->key) {
        free(new_entry);
        return false;
    }
    strcpy(new_entry->key, key);

    new_entry->value1 = value1 & 0x0FFF;
    new_entry->value2 = value2 & 0x0FFF;
    new_entry->next = map->entries[index];
    map->entries[index] = new_entry;
    map->size++;
    if (map->size > map->resize_threshold) {
        hashmap_resize(map);
    }
    return true;
}