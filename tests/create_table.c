#include <stdio.h>

#include "../src/toydb.h"
#include "../src/util.h"
#include "../src/btree.h"

int main(int argc, char const *argv[])
{
    Database db;

    printf("%d\n", db_create("testdb.db", 1));

    printf("%d\n", db_open(&db, "testdb.db"));

    Schema schema;
    schema.n = 0;
    schema.elements[0] = (SchemaElement) { "id", 0 };
    schema.n++;
    schema.elements[1] = (SchemaElement) { "name", 1 };
    schema.n++;
    printf("%d\n", db_create_table(&db, "hello", &schema));

    print_schema(&schema);

    return 0;
}
