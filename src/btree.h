#ifndef BTREE_H
#define BTREE_H


#include <inttypes.h>
#include <stdio.h>


typedef struct Btree {
    FILE *fp;
    int32_t block_size;
    int32_t max_depth;
    int64_t offset;
} Btree;


int btreeii_open(Btree *s, char *filename, int32_t block_size,
                 int32_t max_depth,
                 int do_create, int64_t offset);

int btreeii_insert(Btree *s, int64_t key, int64_t value);

int btreeii_search(Btree *s, int64_t key, int64_t *value);

int btreeii_close(Btree *s);


#endif
