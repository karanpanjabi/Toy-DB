#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "btree.h"


int btree_open(Btree *s, char *filename, int32_t block_size,
                 int32_t max_depth,
                 int do_create, int64_t offset)
{

    /*
        Stores essential details about the B-tree in s. Insert and
            search operations are called using s.
        s: pointer to Btree struct
        filename: File in which B-tree is present
        block_size: Size of blocks read from the file. Every node in
                    the B-tree should occupy one block.
        max_depth: Max allowed depth of the B-tree. This puts the
                   constraint on the max size of the B-tree.
        do_create: Boolean value, 1 if the B-tree to be created,
                   0 if it is already present
        offset: Offset at which the B-tree is found in the file. Value
                ignored if do_create is 1. In this case, empty B-tree
                is appended to file
    */

    

}


int btree_insert(Btree *s, int64_t key, int64_t value)
{

    

}


int btree_search(Btree *s, int64_t key, int64_t *value)
{

    

}


int btree_close(Btree *s)
{

    

}
