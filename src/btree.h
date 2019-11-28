#ifndef BTREE_H
#define BTREE_H


#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>


typedef struct Btree {

    FILE *fp;
    int32_t block_size;
    int32_t max_depth;
    int64_t root_offset;    // where the root node starts [0...max_num_nodes - 1]
    int64_t btree_offset;   // offset in fp where btree struct is stored

    int32_t t;     // elements_per_node is 2*t-1, children per node 2*t
    int32_t pos_last_node;    // pos of last node wrt to 0th node (at location btree_offset + blksize)

} Btree;

int btree_open(Btree *s, FILE *fp, int32_t block_size,
               int32_t max_depth,
               int do_create, int64_t btree_offset);
int btree_insert(Btree *s, int64_t key, int64_t value);
int btree_search(Btree *s, int64_t key, int64_t *value);
int btree_close(Btree *s);

typedef struct Node_Data {
    int64_t key;
    int64_t value_offset;
} Node_Data;

typedef struct Node {
    bool isLeaf;
    int32_t pos;        // linear position wrt to beginning of tree
    int32_t n;
    Node_Data *records;
    int64_t *children_offsets;
} Node;

void init_node(Node *node, int num_records);
void del_node(Node *node);
void write_node(Btree *t, Node *src, int offset);   //write the node src, to the offset into t->fp
void read_node(Btree *t, Node *dst, int offset);    //read the node at the offset to dst from t->fp

#endif
