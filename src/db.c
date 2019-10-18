#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>

#include "db.h"
#include "btree.h"


int db_create(char *dbname)
{

    /*
        Creates a file of name dbname and initializes metadata and
            directory B-tree.

        dbname: Name of database file to be created

        Return value:
            0: Success
            1: File exists
            2: fopen or stat failed, errno set
            3: Write error
    */

    FILE *fp;
    struct stat dbfile_stat;
    int32_t block_size;

    if (access(dbname, F_OK) == 0) {
        return 1;
    }

    fp = fopen(dbname, "w+");
    if (fp == NULL) {
        return 2;
    }

    if (stat(dbname, &dbfile_stat) == -1) {
        return 2;
    }

    block_size = dbfile_stat.st_blksize;

    // TODO: Initialize file

    fclose(fp);

    return 0;

}


int db_open(Database *db, char *dbname)
{

    /*
        Open the database file dbname and store data in db

        db: (Database *) where the opened DB info will be stored
        dbname: Name of database file to open

        Return Value:
            0: Success
            1: fopen failed, errno set
            2: fread failed. Caller should call ferror and feof.
    */

    if ((db->fp = fopen(dbname, "r+")) == NULL) {
        return 1;
    }

    if (fread(&(db->block_size), sizeof(int32_t), 1, db->fp) == 0) {
        return 2;
    }

    if (fread(&(db->max_depth), sizeof(int32_t), 1, db->fp) == 0) {
        return 2;
    }

    return 0;

}


// TODO: Accept schema too
int db_create_table(Database *db, char *tablename)
{

    /*
        Creates table tablename in Database db.

        db: (Database *) on which operations are performed
        tablename: String with the tablename to be created
    */

    

}


int db_insert(Database *db, char *tablename, ...)
{

    /*
        
    */

    

}


int db_select(Database *db, char *tablename,
              int n_cols, char *column_names[],
              int where_clause, char *lhs,
              int is_rhs_const, char *rhs)
{

    /*
        
    */

    

}


int db_close(Database *db)
{

    /*
        
    */

    

}
