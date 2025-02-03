#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Entry {
    char* key;
    uint16_t value1;
    uint16_t value2;
    struct Entry* next;
} Entry;

typedef struct {
    Entry** entries;
    size_t capacity;
    size_t size;
} HashMap;

HashMap* hashmap_create(size_t capacity);
void hashmap_destroy(HashMap* map);
bool hashmap_set(HashMap* map, const char* key, uint16_t value1, uint16_t value2);
bool hashmap_get(HashMap* map, const char* key, uint16_t* value1, uint16_t* value2);
bool hashmap_delete(HashMap* map, const char* key);

#endif // HASHMAP_H