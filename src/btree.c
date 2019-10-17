#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define NODE_METADATA_SIZE (16)

#include "block.h"
#include "btree.h"


int btree_open(Btree *s, char *filename,
                 int do_create, int64_t offset)
{

    /*
        Constructor Method:
            Stores essential details about the B-tree in s. Insert and
            search operations are called using s.
        s: pointer to Btree struct. s must be allocated by caller.
        filename: File in which B-tree is present
        block_size: Size of blocks read from the file. Every node in
                    the B-tree should occupy one block.
        max_depth: Max allowed depth of the B-tree. This puts the
                   constraint on the max size of the B-tree.
        do_create: Boolean value, non-zero if the B-tree to be
                   created, 0 if it is already present
        offset: Offset at which the B-tree is found in the file. Value
                ignored if do_create is non-zero. In this case, empty
                B-tree is appended to file

        Return Value:
            0: Success
            1: Opening filename failed. fopen sets errno.
            2: Metadata read error
            3: fseek or ftell failed and will set errno
            4: do_create failed
    */

    int n_elems_per_node;
    char empty_btree_fill_byte;
    Btree directory;

    s->fp = fopen(filename, "r+");
    if (s->fp == NULL) {
        return 1;
    }

    if (fread(&(s->block_size), sizeof(int32_t), 1, s->fp) < 1) {
        return 2;
    }
    if (fread(&(s->max_depth), sizeof(int32_t), 1, s->fp) < 1) {
        return 2;
    }

    n_elems_per_node = (s->block_size - NODE_METADATA_SIZE)
                        / (sizeof(int64_t) * 2);

    if (do_create != 0) {

        empty_btree_fill_byte = '\0';

        if (fseek(s->fp, 0, SEEK_END) == -1) {
            return 3;
        }
        offset = ftell(s->fp);
        if (offset == -1) {
            return 3;
        }

        directory = *s;

        // B-tree directory is stored in Block 1
        directory.offset = directory.block_size;

        // TODO: Update directory B-tree to store offset of new B-tree

        // TODO: Append empty B-tree to end of s->fp

    }

    s->offset = offset;

    return 0;

}


int btree_insert(Btree *s, int64_t key, int64_t value)
{

    /*
        Insert the given key and value into the B-tree given by s

        s: Pointer to Btree
        key: Key to be stored in the B-tree
        value: Value corresponding to the key to be stored
    */

    

}


int btree_search(Btree *s, int64_t key, int64_t *value)
{

    /*
        Search for the given key in the B-tree given by s and store
            the return value in the integer pointed to by value

        s: Pointer to Btree
        key: Key to search for in the B-tree
        value: Pointer to integer where the value correspoinding to
               key will be stored
    */

    

}


int btree_close(Btree *s)
{

    /*
        Destructor Method:
            Free resources corresponding to s. Freeing memory
            allocated for *(s) using malloc family of functions must
            be freed by the caller.

        Return Value:
            0: Success
            1: fclose failed. fclose will set errno.
    */

    if (fclose(s->fp) == EOF) {
        return 1;
    }

    return 0;

}
