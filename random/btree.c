#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NODE_METADATA_SIZE (16)

#define BLKROUNDDOWN(adr, blksize) ((adr / blksize) * blksize)
#define BLKROUNDUP(adr, blksize) (BLKROUNDDOWN(adr, blksize) + blksize)

#include "header.h"

static int node_size;

char buffer[4096];

long get_num_nodes(Btree *s);
void update_btree_header(Btree *s);

int btree_open(Btree *s, FILE *fp, int32_t block_size,
               int32_t max_depth,
               int do_create, int64_t btree_offset)
{

    /*
        Constructor Method:
            Stores essential details about the B-tree in s. Insert and
            search operations are called using s.
        s: pointer to Btree struct. s must be allocated by caller.
        fp: File pointer to filein which B-tree is present
        do_create: Boolean value, non-zero if the B-tree to be
                   created, 0 if it is already present
        btree_offset: Offset at which the B-tree is found in the file.
                     Value ignored if do_create is non-zero. In this
                     case, empty B-tree is appended to file

        Return Value:
            0: Success
            2: Metadata read error
            3: fseek or ftell failed and will set errno
            4: do_create failed
    */

    int n_elems_per_node;
    char empty_btree_fill_byte;

    s->fp = fp;
    s->block_size = block_size;
    s->max_depth = max_depth;

    // to find out number of (actual) elements per node, remove the metadata part
    n_elems_per_node = (s->block_size - (sizeof(Node) - sizeof(Node_Data *) - sizeof(int64_t *)) - sizeof(int64_t)) / (sizeof(Node_Data) + sizeof(int64_t));

    // to make sure 2*t - 1 = num of elements per node
    if (n_elems_per_node % 2 == 0)
    {
        n_elems_per_node--;
    }

    s->t = (n_elems_per_node + 1) / 2;

    // create the btree in the file, ignore root_offset arg, create root node
    if (do_create != 0)
    {

        s->pos_last_node = 0;
        empty_btree_fill_byte = '\0';

        if (fseek(s->fp, 0, SEEK_END) == -1)
        {
            return 3;
        }
        
        int btree_pos = ftell(s->fp);
        btree_pos = btree_pos % block_size != 0 ? BLKROUNDUP(btree_pos, block_size) : btree_pos;
        fseek(s->fp, btree_pos, SEEK_SET);
        s->btree_offset = ftell(s->fp);

        // Find out the offset of the root node
        s->root_offset = 0;
        
        // first write the tree struct, then write the root node
        fwrite(s, sizeof(Btree), 1, s->fp);

        fseek(s->fp, BLKROUNDUP(ftell(s->fp), s->block_size), SEEK_SET); // seek to next block (root node lives here)

        // seek to the end of the tree to reserve space
        long max_num_nodes = get_num_nodes(s);
        fseek(s->fp, max_num_nodes * block_size - 1, SEEK_CUR);
        printf("Num nodes: %ld\n", max_num_nodes);
        printf("Elements per node: %d\n", n_elems_per_node);
        printf("Elements per tree: %ld\n", n_elems_per_node * max_num_nodes);
        printf("End of tree: %ld\n", ftell(s->fp));
        fwrite("0", 1, 1, s->fp);

        // go back to write root
        fseek(s->fp, BLKROUNDUP(ftell(s->fp), s->block_size), SEEK_SET); // seek to next block (root node lives here)
        Node root;
        init_node(&root, 2 * s->t - 1);
        root.pos = 0;
        write_node(s, &root, root.pos);
        // free records and children_offset arrays in root
        del_node(&root);
    }
    else
    {
        // read btree at btree_offset?
        Btree temptree;
        fseek(fp, btree_offset, SEEK_SET);
        fread(&temptree, sizeof(Btree), 1, fp);
        s->t = temptree.t;
        s->root_offset = temptree.root_offset;
        s->pos_last_node = temptree.pos_last_node;
        s->btree_offset = temptree.btree_offset;
    }

    // s->root_offset = root_offset;

    node_size = sizeof(Node) - sizeof(Node_Data *) - sizeof(int64_t *) +
                sizeof(Node_Data) * (2 * s->t - 1) +
                sizeof(int64_t) * (2 * s->t);

    // printf("func t: %d\n", s->t);

    return 0;
}

long get_num_nodes(Btree *s)
{
    long max_num_nodes = 0;
    int a = 2 * s->t;
    long res = 1;
    for (int r = 0; r <= s->max_depth; r++)
    {
        max_num_nodes += res;
        res = res * a;
    }
    return max_num_nodes;
}

/*
Btree* btree_open_file(FILE *fp, int offset) {
    Btree *tree = (Btree *) malloc(sizeof(Btree));

    fseek(fp, 0, 0);
    fread(tree, sizeof(Btree), 1, fp);

    tree->fp = fp;

    return tree;
}
*/

int clrs_btree_search(Btree *s, Node *x, int64_t key, int64_t *value)
{
    int i = 0;
    while (i < x->n && key > x->records[i].key)
    {
        i++;
    }

    if (i < x->n && key == x->records[i].key)
    {
        *value = x->records[i].value_offset;
        return 0;
    }
    else if (x->isLeaf)
    {
        return -1;
    }
    else
    {
        Node xci; 
        init_node(&xci, 2 * s->t - 1); // init node
        read_node(s, &xci, x->children_offsets[i]); // figure out the offset of the child i
        int ret = clrs_btree_search(s, &xci, key, value);
        del_node(&xci); // free up arrays of xci
        return ret;
    }
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

    Node root;
    init_node(&root, 2 * s->t - 1);
    read_node(s, &root, s->root_offset); // read the node at this into a node struct

    int ret = clrs_btree_search(s, &root, key, value);
    del_node(&root);

    return ret;
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

    // if (fclose(s->fp) == EOF)
    // {
    //     return 1;
    // }

    return 0;
}

void init_node(Node *node, int num_records)
{
    node->isLeaf = true;
    node->n = 0;
    node->pos = 0;
    node->records = (Node_Data *)malloc(num_records * sizeof(Node_Data));
    node->children_offsets = (int64_t *)malloc((num_records + 1) * sizeof(int64_t));
    for (int i = 0; i < num_records + 1; i++)
    {
        node->children_offsets[i] = -1;
    }
}

void del_node(Node *node)
{
    if (node != NULL)
    {
        free(node->records);
        free(node->children_offsets);
    }
}

// offset is which n_th node from 0: root is 0, leftmost child is 1
void write_node(Btree *t, Node *src, int offset)
{
    // write isLeaf, pos, and n and then write the records array
    // TODO: use block write to write to wherever
    printf("Btree offset: %ld, node write offset: %ld\n", t->btree_offset, t->btree_offset + (offset+1) * t->block_size);
    fseek(t->fp, t->btree_offset + (offset+1) * t->block_size, SEEK_SET);
    fwrite(&src->isLeaf, sizeof(bool), 1, t->fp);
    fwrite(&src->pos, sizeof(int), 1, t->fp);
    fwrite(&src->n, sizeof(int), 1, t->fp);
    fwrite(src->records, sizeof(Node_Data), 2 * t->t - 1, t->fp);
    fwrite(src->children_offsets, sizeof(int64_t), 2 * t->t, t->fp);
}

void read_node(Btree *t, Node *dst, int offset)
{
    // read isLeaf, pos,  then read the records array
    fseek(t->fp, t->btree_offset + (offset+1) * t->block_size, SEEK_SET);    // fseek from btree_offset, offset: 0 is first node after inserting btree struct
    fread(&dst->isLeaf, sizeof(bool), 1, t->fp);
    fread(&dst->pos, sizeof(int), 1, t->fp);
    fread(&dst->n, sizeof(int), 1, t->fp);
    fread(dst->records, sizeof(Node_Data), 2 * t->t - 1, t->fp); // TODO: the records memory should be malloc-d before this
    fread(dst->children_offsets, sizeof(int64_t), 2 * t->t, t->fp);
}

void btree_split_child(Btree *s, Node *x, int i, Node *y)
{

    if (s->pos_last_node == get_num_nodes(s) - 1)
        return;

    Node zz;
    Node *z = &zz;
    init_node(z, 2 * s->t - 1);

    s->pos_last_node++;
    z->pos = s->pos_last_node;

    z->isLeaf = y->isLeaf;
    z->n = s->t - 1; // half of what y had (t-1)

    int j;

    // copy the records
    for (j = 0; j < s->t - 1; j++)
    {
        z->records[j] = y->records[j + s->t];
    }

    if (!y->isLeaf)
    {
        // copy the children offsets if y isn't a leaf
        for (j = 0; j < s->t; j++)
        {
            z->children_offsets[i] = y->children_offsets[j + s->t];
        }
    }

    y->n = s->t - 1;

    // move the children forward
    for (j = x->n; j >= i + 1; j--)
    {
        x->children_offsets[j + 1] = x->children_offsets[j];
    }

    // add z as child (i+1) to x
    x->children_offsets[i + 1] = z->pos;

    // move the records in x
    for (j = x->n - 1; j >= i; j--)
    {
        x->records[j + 1] = x->records[j];
    }

    x->records[i] = y->records[s->t - 1];

    x->n += 1;

    write_node(s, x, x->pos);
    write_node(s, y, y->pos);
    write_node(s, z, z->pos);
    update_btree_header(s);    // because pos_last_node increased

    del_node(z);
}

int clrs_insert_non_full(Btree *s, Node *node, Node_Data *data_record)
{
    int i = (node->n) - 1;

    if(node->isLeaf)
    {
        while (i>=0 && node->records[i].key > data_record->key)
        {
            node->records[i+1] = node->records[i];
            i--;
        }

        if(node->records[i].key == data_record->key)
        {
            // duplicate key
            return -1;
        }

        node->records[i+1] = *data_record;
        node->n++;
        write_node(s, node, node->pos);
    }
    else
    {
        while (i>=0 && node->records[i].key > data_record->key)
        {
            i--;
        }
        i++;

        Node xx_ci; Node *x_ci = &xx_ci;
        init_node(x_ci, 2*s->t - 1);
        read_node(s, x_ci, node->children_offsets[i]);

        if(x_ci->n == 2*s->t - 1)
        {
            btree_split_child(s, node, i, x_ci);
            if(data_record->key > node->records[i].key)     // TODO: check why this condition is used
            {
                i++;
                read_node(s, x_ci, node->children_offsets[i]);
            }
        }

        return clrs_insert_non_full(s, x_ci, data_record);

        del_node(x_ci);
    }
    
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


    Node_Data data_record = {key, value};

    Node rroot; Node *root = &rroot;
    init_node(root, 2*s->t - 1);
    read_node(s, root, s->root_offset);

    if(root->n == 2*s->t-1)
    {
        if (s->pos_last_node == get_num_nodes(s) - 1)
            return -1;

        Node rroot_new; 
        Node *root_new = &rroot_new;
        init_node(root_new, 2*s->t - 1);

        root_new->isLeaf = false;
        root_new->children_offsets[0] = root->pos;
        root_new->n = 0;

        s->pos_last_node++;
        root_new->pos = s->pos_last_node;
        s->root_offset = root_new->pos;

        update_btree_header(s);

        btree_split_child(s, root_new, 0, root);
        if(clrs_insert_non_full(s, root_new, &data_record) == -1)
        {
            return -1;
        }

        del_node(root_new);
    }
    else
    {
        if(clrs_insert_non_full(s, root, &data_record) == -1)
        {
            return -1;
        }
    }
    

    del_node(root);
}

void update_btree_header(Btree *s)
{
    fseek(s->fp, s->btree_offset, SEEK_SET);
    fwrite(s, sizeof(Btree), 1, s->fp);
}