#include <stdio.h>
#include <string.h>

#include "btree.h"
#include "toydb.h"

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
        printf("%s %ld %ld, ", (char *)&(ptr->records[i].key), ptr->records[i].key, ptr->records[i].value_offset);
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

// extremely bad code, many disk accesses
void print_data_schema(int64_t data_offset, Schema *schema, Btree *s)
{
    static int strsize = 256;
    static char outbuff[256];

    fseek(s->fp, data_offset, SEEK_SET);
    for (int i = 0; i < schema->n; i++)
    {
        switch (schema->elements[i].dtype)
        {
            case 0:
                ;
                fread(outbuff, sizeof(int64_t), 1, s->fp);
                printf("%ld\t", * (int64_t *) outbuff);
                break;
            case 1:
                ;
                int64_t ipos = ftell(s->fp);
                fread(outbuff, sizeof(char), strsize, s->fp);
                int stread = strlen(outbuff) + 1;
                fseek(s->fp, ipos + stread, SEEK_SET);

                printf("%s\t", outbuff);
                break;
            case 2:
                ;
                fread(outbuff, sizeof(float), 1, s->fp);
                printf("%f\t", * (float *) outbuff);
                break;
        }
    }
}

void print_node_data(Node *ptr, Schema *schema, Btree *s)
{
    // printf("-------Node start-------\n");
    // printf("isLeaf: %d, pos: %d, n: %d\n", ptr->isLeaf, ptr->pos, ptr->n);
    // printf("Records: [");
    for (int i = 0; i < ptr->n; i++)
    {
        printf("%ld %ld, ", ptr->records[i].key, ptr->records[i].value_offset);
        print_data_schema(ptr->records[i].value_offset, schema, s);
    }
    // printf("]\n");

    // printf("Children: [");
    // for (int i = 0; i < ptr->n + 1; i++)
    // {
    //     printf("%ld, ", ptr->children_offsets[i]);
    // }
    // printf("]\n\n");
    // printf("-------Node end-------\n");
}

void display_nodes_recurse(Btree *s, Node *node)
{
    print_node(node);

    if(!node->isLeaf)
    {
        for (int i = 0; i <= node->n; i++)
        {
            Node child;
            init_node(&child, 2*s->t - 1);
            read_node(s, &child, node->children_offsets[i]);
            display_nodes_recurse(s, &child);
            del_node(&child);
        }
        
    }
}

void display_nodes_recurse_data(Btree *s, Node *node, Schema *schema)
{
    print_node_data(node, schema, s);

    if(!node->isLeaf)
    {
        for (int i = 0; i <= node->n; i++)
        {
            Node child;
            init_node(&child, 2*s->t - 1);
            read_node(s, &child, node->children_offsets[i]);
            display_nodes_recurse_data(s, &child, schema);
            del_node(&child);
        }
        
    }
}

void recurse_tree(Btree *s)
{
    Node root;
    init_node(&root, 2*s->t - 1);

    read_node(s, &root, s->root_offset);
    display_nodes_recurse(s, &root);

    del_node(&root);
}

void recurse_tree_data(Btree *s, Schema *schema)
{
    Node root;
    init_node(&root, 2*s->t - 1);

    read_node(s, &root, s->root_offset);
    display_nodes_recurse_data(s, &root, schema);

    del_node(&root);
}

void print_dir_btree(Database *db)
{
    fseek(db->fp, db->block_size, SEEK_SET);
    Btree directory;
    btree_open(&directory, db->fp, db->block_size, db->max_depth, 0, db->block_size);
    disp_btree(&directory);
    recurse_tree(&directory);
}

void print_schema(Schema *schema)
{
    for (int i = 0; i < schema->n; i++)
    {
        switch(schema->elements[i].dtype)
        {
            case 0:
                ;
                printf("%s int\n", schema->elements[i].fieldname);
                break;
            case 1:
                ;
                printf("%s str\n", schema->elements[i].fieldname);
                break;
            case 2:
                ;
                printf("%s float\n", schema->elements[i].fieldname);
                break;
        }
    }
}

void read_schema(Database *db, char *table)
{
    fseek(db->fp, db->block_size, SEEK_SET);
    Btree directory; Btree *dirbtree = &directory;
    btree_open(&directory, db->fp, db->block_size, db->max_depth, 0, db->block_size);
    
    int64_t translated_tablename = *(int64_t *) table;

    int64_t schema_offset;
    if(btree_search(dirbtree, translated_tablename, &schema_offset) != -1)
        return;

    Schema s;
    fseek(dirbtree->fp, schema_offset, SEEK_SET);
    fwrite(&s, sizeof(Schema), 1, dirbtree->fp);

    print_schema(&s);
    
}