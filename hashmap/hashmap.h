#ifndef HASHMAP_H
#define HASHMAP_H

#include <wchar.h>

typedef struct HashNode {
  wchar_t *key;
  char *value;
  struct HashNode *next;
} HashNode;

typedef struct HashMap {
  HashNode **buckets;
  int size;
} HashMap;

extern int no_ident;

HashMap *create_hashmap(int size);
void insert(HashMap *map, const wchar_t *key);
char *get(HashMap *map, const wchar_t *key);
void free_hashmap(HashMap *map);
unsigned long hash_wchar(const wchar_t *str);

#endif
