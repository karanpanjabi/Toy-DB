#include <stdio.h>

#include "../header.h"

void disp_btree(Btree *t)
{
    printf("-------BTree header-------\n");
    printf("Block size: %d\n", t->block_size);
    printf("Max depth: %d\n", t->max_depth);
    printf("Btree offset: %ld\n", t->btree_offset);
    printf("Root offset: %ld\n", t->root_offset);
    printf("t: %d\n", t->t);
    printf("pos_last_node: %d\n", t->pos_last_node);
    printf("-------End BTree header-------\n\n");
}

void print_node(Node *ptr)
{
    printf("-------Node start-------\n");
    printf("isLeaf: %d, pos: %d, n: %d\n", ptr->isLeaf, ptr->pos, ptr->n);
    printf("Records: [");
    for (int i = 0; i < ptr->n; i++)
    {
        printf("%ld %ld, ", ptr->records[i].key, ptr->records[i].value_offset);
    }
    printf("]\n");

    printf("Children: [");
    for (int i = 0; i < ptr->n + 1; i++)
    {
        printf("%ld, ", ptr->children_offsets[i]);
    }
    printf("]\n\n");
    printf("-------Node end-------\n");
}