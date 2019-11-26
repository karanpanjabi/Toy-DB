#include <stdio.h>
#include "../header.h"

int main()
{
    Btree tree;
    FILE *fp = fopen("tree.dat", "w+");
    btree_open(&tree, fp, 128, 1, 1, -1);
}
