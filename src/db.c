#include <inttypes.h>
#include <stdio.h>

#include "db.h"
#include "btree.h"


int db_create(char *dbname)
{

    

}


int db_open(Database *db, char *dbname)
{

    

}


int db_create_table(Database *db, char *tablename)
{

    

}


int db_insert(Database *db, char *tablename, ...)
{

    

}


int db_select(Database *db, char *tablename,
              int n_cols, char *column_names[],
              int where_clause, char *lhs,
              int is_rhs_const, char *rhs)
{

    

}


int db_close(Database *db)
{

    

}
