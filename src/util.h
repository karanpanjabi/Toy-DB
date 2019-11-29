#include "btree.h"
#include "toydb.h"

void recurse_tree(Btree *s);
void recurse_tree_data(Btree *s, Schema *schema);
void print_dir_btree(Database *db);
void read_schema(Database *db, char *table);
void print_schema(Schema *schema);