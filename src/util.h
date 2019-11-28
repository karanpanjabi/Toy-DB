#include "btree.h"
#include "toydb.h"

void recurse_tree(Btree *s);
void recurse_tree_data(Btree *s, Schema *schema);