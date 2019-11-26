#include <stdio.h>
#include "../header.h"

int main()
{
    Btree tree; Btree *tree_ptr = &tree;
    FILE *fp = fopen("tree.dat", "r+");
    btree_open(&tree, fp, 128, 1, 0, 0);

    btree_insert(tree_ptr, 1234, 6789);
    btree_insert(tree_ptr, 2222, 67829);
    btree_insert(tree_ptr, 134, 9999);

    btree_insert(tree_ptr, 189, 3333);
    btree_insert(tree_ptr, 200, 789);

    printf("%d\n", btree_insert(tree_ptr, 210, 789));

    printf("%d\n", btree_insert(tree_ptr, 210, 789));

    fclose(fp);
}
