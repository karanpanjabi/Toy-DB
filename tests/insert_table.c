#include <stdio.h>

#include "../src/toydb.h"
#include "../src/util.h"
#include "../src/btree.h"

int main(int argc, char const *argv[])
{
    Database db;

    printf("db_open returned: %d\n", db_open(&db, "testdb.db"));

    char *tablename = "hello";

    int64_t id = 23;
    char *st = "this is an entry";

    DataRecord row[2];
    row[0] = (DataRecord) {
        0,
        (void *) &id
    };
    row[1] = (DataRecord) {
        1,
        (void *) st
    };

    printf("db_insert returned: %d\n", db_insert(&db, tablename, row, 2));

    id = 44;
    st = "this is another entry";
    row[0] = (DataRecord) {
        0,
        (void *) &id
    };
    row[1] = (DataRecord) {
        1,
        (void *) st
    };

    printf("db_insert returned: %d\n", db_insert(&db, tablename, row, 2));


    return 0;
}
