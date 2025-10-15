#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "hashmap.h"

int no_ident = 0;

char* strdup_portable(const char* s) {
    char* dup = malloc(strlen(s) + 1);
    if (dup) strcpy(dup, s);
    return dup;
}

wchar_t* wcsdup_portable(const wchar_t* s) {
    size_t len = wcslen(s) + 1;
    wchar_t* dup = malloc(len * sizeof(wchar_t));
    if (dup) wcscpy(dup, s);
    return dup;
}

unsigned long hash_wchar(const wchar_t* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

HashMap* create_hashmap(int size) {
    HashMap* map = malloc(sizeof(HashMap));
    map->size = size;
    map->buckets = calloc(size, sizeof(HashNode*));
    return map;
}

char* get(HashMap* map, const wchar_t* key) {
    unsigned long hash = hash_wchar(key);
    int index = hash % map->size;
    HashNode* node = map->buckets[index];
    while (node) {
        if (wcscmp(node->key, key) == 0) return node->value;
        node = node->next;
    }
    return NULL;
}

void insert(HashMap* map, const wchar_t* key) {
    unsigned long hash = hash_wchar(key);
    int index = hash % map->size;

    char value[50];
    if(get(map,key) != NULL) return;
    sprintf(value, "_var%d", no_ident++);
    
    HashNode* node = map->buckets[index];
    while (node) {
        if (wcscmp(node->key, key) == 0) {
            free(node->value);
            node->value = strdup_portable(value);
            return;
        }
        node = node->next;
    }

    HashNode* new_node = malloc(sizeof(HashNode));
    new_node->key = wcsdup_portable(key);
    new_node->value = strdup_portable(value);
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
}

void free_hashmap(HashMap* map) {
    for (int i = 0; i < map->size; i++) {
        HashNode* node = map->buckets[i];
        while (node) {
            HashNode* temp = node;
            node = node->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}
