#include <stdio.h>
#include "../header.h"

int main()
{
    Btree tree; 
    FILE *fp = fopen("tree.dat", "r");
    btree_open(&tree, fp, 128, 1, 0, 0);

    int64_t val;
    printf("Found: %d\n", btree_search(&tree, 189, &val));
    printf("Val: %ld\n", val);

    printf("Found: %d\n", btree_search(&tree, 210, &val));
    printf("Val: %ld\n", val);

    printf("Found: %d\n", btree_search(&tree, 888, &val));
    printf("Val: %ld\n", val);
}