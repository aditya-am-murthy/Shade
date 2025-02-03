#include "hashmap.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

// Hash function (djb2 algorithm)
static size_t hash(const char* key, size_t capacity) {
    size_t hash_value = 5381;
    int c;
    while ((c = *key++)) {
        hash_value = ((hash_value << 5) + hash_value) + c; // hash * 33 + c
    }
    return hash_value % capacity;
}

// Check if a number is prime
static bool is_prime(size_t n) {
    if (n < 2) return false;
    for (size_t i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return false;
    }
    return true;
}

// Find the next prime number >= n
static size_t next_prime(size_t n) {
    while (!is_prime(n)) {
        n++;
    }
    return n;
}

// Resize the hashmap when load factor exceeds 70%
static void hashmap_resize(HashMap* map) {
    size_t new_capacity = next_prime(map->capacity * 2);
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

// Create a hashmap with initial prime capacity
HashMap* hashmap_create(size_t capacity) {
    HashMap* map = malloc(sizeof(HashMap));
    if (!map) return NULL;

    map->capacity = next_prime(capacity);
    map->entries = calloc(map->capacity, sizeof(Entry*));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    map->size = 0;
    map->resize_threshold = (size_t)(map->capacity * 0.7);
    return map;
}

// Destroy the hashmap and free all memory
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

// Insert or update a key with integer and float values
bool hashmap_set(HashMap* map, const char* key, uint16_t value1, uint16_t value2, float latitude, float longitude) {
    size_t index = hash(key, map->capacity);
    Entry* entry = map->entries[index];

    // Check for existing key and update
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value1 = value1 & 0x0FFF; // 12-bit mask
            entry->value2 = value2 & 0x0FFF;
            entry->latitude = latitude;
            entry->longitude = longitude;
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
    new_entry->latitude = latitude;
    new_entry->longitude = longitude;
    new_entry->next = map->entries[index];
    map->entries[index] = new_entry;
    map->size++;

    // Resize if load factor exceeds threshold
    if (map->size > map->resize_threshold) {
        hashmap_resize(map);
    }
    return true;
}

// Retrieve values for a key
bool hashmap_get(HashMap* map, const char* key, uint16_t* value1, uint16_t* value2, float* latitude, float* longitude) {
    size_t index = hash(key, map->capacity);
    Entry* entry = map->entries[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            *value1 = entry->value1;
            *value2 = entry->value2;
            *latitude = entry->latitude;
            *longitude = entry->longitude;
            return true;
        }
        entry = entry->next;
    }
    return false;
}

// Delete a key-value pair
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