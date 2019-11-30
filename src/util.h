#include "btree.h"
#include "toydb.h"

#define MIN(a, b) ((b) < (a) ? (b) : (a))

void recurse_tree(Btree *s);
void recurse_tree_data(Btree *s, Schema *schema);
void print_dir_btree(Database *db);
void read_table_data(Database *db, char *table);
void print_schema(Schema *schema);