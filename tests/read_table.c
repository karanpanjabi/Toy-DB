#include <stdio.h>

#include "../src/toydb.h"
#include "../src/util.h"
#include "../src/btree.h"

int main(int argc, char const *argv[])
{
    Database db;

    printf("db_open returned: %d\n", db_open(&db, "testdb.db"));

    read_table_data(&db, "hello");

    return 0;
}
