#ifndef DB_H
#define DB_H

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>


typedef struct Database {

    FILE *fp;
    int32_t block_size;
    int32_t max_depth;

} Database;


typedef struct SchemaElement {

    /*
        dtype:
            0 -> int64_t
            1 -> string (char *)
            2 -> float? (precision)?
    */

    char fieldname[8];
    int32_t dtype;

} SchemaElement;


int db_create(char *dbname, int32_t max_depth);
int db_open(Database *db, char *dbname);
int db_create_table(Database *db, char *tablename,
                    SchemaElement *schema, int n_schemaelements);
int db_insert(Database *db, char *tablename, ...);
int db_select(Database *db, char *tablename,
              int n_cols, char *column_names[],
              int where_clause, char *lhs,
              int is_rhs_const, char *rhs);
int db_close(Database *db);


#endif
