#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Entry {
    char* key;                  // Key (string)
    uint16_t value1;            // First 12-bit integer
    uint16_t value2;            // Second 12-bit integer
    float latitude;             // Latitude (float)
    float longitude;            // Longitude (float)
    struct Entry* next;         // Pointer to next entry
} Entry;

typedef struct {
    Entry** entries;            // Array of entry pointers
    size_t capacity;            // Total capacity
    size_t size;                // Current number of entries
    size_t resize_threshold;    // Resize threshold (70% load factor)
} HashMap;

// Function prototypes
HashMap* hashmap_create(size_t capacity);
void hashmap_destroy(HashMap* map);
bool hashmap_set(HashMap* map, const char* key, uint16_t value1, uint16_t value2, float latitude, float longitude);
bool hashmap_get(HashMap* map, const char* key, uint16_t* value1, uint16_t* value2, float* latitude, float* longitude);
bool hashmap_delete(HashMap* map, const char* key);

#endif // HASHMAP_H