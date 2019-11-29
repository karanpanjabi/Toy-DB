#include <stdio.h>

#include "../src/toydb.h"
#include "../src/util.h"
#include "../src/btree.h"

int main(int argc, char const *argv[])
{
    Database db;

    printf("%d\n", db_open(&db, "testdb.db"));

    print_dir_btree(&db);

    return 0;
}
