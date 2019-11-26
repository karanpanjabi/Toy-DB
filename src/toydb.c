#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "toydb.h"
#include "btree.h"
#include "block.h"


int db_create(char *dbname, int32_t max_depth)
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
            4: B-tree error
    */

    int r;

    FILE *fp;
    struct stat dbfile_stat;
    int32_t block_size;
    int64_t bytes_written;
    int32_t fw_rv;
    int ret;
    Btree directory;

    if (access(dbname, F_OK) == 0) {
        r = 1;
        goto FILE_EXISTS;
    }

    fp = fopen(dbname, "w+");
    if (fp == NULL) {
        r = 2;
        goto FOPEN_FAILED;
    }

    if (stat(dbname, &dbfile_stat) == -1) {
        r = 2;
        goto STAT_FAILED;
    }

    block_size = dbfile_stat.st_blksize;

    bytes_written = 0;

    if ((fw_rv = fwrite(&block_size, sizeof(int32_t), 1, fp)) == 1) {
        r = 3;
        goto FWRITE_FAILED;
    }
    bytes_written += fw_rv;

    if ((fw_rv = fwrite(&max_depth, sizeof(int32_t), 1, fp)) == 1) {
        r = 3;
        goto FWRITE_FAILED;
    }
    bytes_written += fw_rv;

    if ((fw_rv = fwrite("\0", sizeof(char),
                            (block_size - bytes_written), fp))
             < (block_size - bytes_written)) {
        r = 3;
        goto FWRITE_FAILED;
    }

    ret = btree_open(&directory, fp, block_size, max_depth, 1, 0);
    if (ret != 0) {
        r = 4;
        goto BTREE_ERROR;
    }
    ret = btree_close(&directory);
    if (ret != 0) {
        r = 4;
        goto BTREE_ERROR;
    }

    r = 0;
    goto SUCCESS;

    SUCCESS:
    BTREE_ERROR:
    FWRITE_FAILED:
    STAT_FAILED:

    fclose(fp);

    FOPEN_FAILED:
    FILE_EXISTS:

    return r;

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


int db_create_table(Database *db, char *tablename,
                    SchemaElement *schema, int n_schemaelements)
{

    /*
        Creates table tablename in Database db.

        db: (Database *) on which operations are performed
        tablename: String with the tablename to be created
    */

    int r, v;
    int i, j;
    int64_t ret;
    int64_t translated_tablename;
    char *dtype;
    Block block;
    Btree directory, table_index;

    // generic failure return value
    r = 1;

    /* First schema element must be int64_t.
       It will be the primary key.
       If it is not an int64_t, return 2 to indicate this failure.  */
    if (schema[0].dtype != 0) {
        r = 2;
        goto ERR_INVALID_SCHEMA;
    }

    memset(&translated_tablename, 0, sizeof(int64_t));
    memcpy(&translated_tablename, tablename, strlen(tablename));

    v = btree_open(&directory, db->fp, db->block_size, db->max_depth,
                   0, db->block_size);
    if (v != 0) {
        r = 4;
        goto ERR_DIRECTORY_OPEN;
    }

    btree_insert(&directory, translated_tablename,
                 (int64_t)fseek(db->fp, 0, SEEK_END));

    block.block_size = db->block_size;
    block.n_occupied = 0;
    block.block = malloc(db->block_size * sizeof(char));
    if (block.block == NULL) {
        r = 1;
        goto ERR_BLOCK_MALLOC;
    }

    for (i = 0; i < n_schemaelements; i++) {

        if (block.block_size - block.n_occupied <
                sizeof(SchemaElement)) {
            r = 3;
            goto ERR_CREATE;
        }
        ret = snprintf(block.block, 8, "%s", schema[i].fieldname);
        if (ret >= 8) {
            r = 3;
            goto ERR_CREATE;
        } else {
            block.n_occupied += ret;
        }

        dtype = (char *)(&(schema[i].dtype));
        for (j = 0; j < sizeof(int32_t); j++)
            block.block[block.n_occupied + j] = dtype[j];
        block.n_occupied += sizeof(int32_t);

    }

    memset(block.block + block.n_occupied, 0,
           block.block_size - block.n_occupied);
    block_append(&block, db->fp);

    v = btree_open(&table_index, db->fp, db->block_size,
                   db->max_depth, 1, 0);
    if (v != 0) {
        r = 4;
        goto ERR_BTREE_CREATE;
    }
    v = btree_close(&table_index);
    if (v != 0) {
        r = 4;
        goto ERR_BTREE_CREATE;
    }

    ERR_CREATE:
    ERR_BTREE_CREATE:

        free(block.block);
    ERR_BLOCK_MALLOC:

        v = btree_close(&directory);
        if (v != 0) {
            r = 4;
        }
    ERR_DIRECTORY_OPEN:
    ERR_INVALID_SCHEMA:

    return r;

}


int db_insert(Database *db, char *tablename, ...)
{

    /*
        
    */

    int r, v;
    int i, j;
    int64_t index_offset;
    int64_t ret;
    int64_t translated_tablename;
    char *dtype;
    Block block;
    Btree directory, table_index;

    // generic failure return value
    r = 1;

    v = btree_open(&directory, db->fp, db->block_size, db->max_depth,
                   0, db->block_size);
    if (v != 0) {
        r = 4;
        goto ERR_DIRECTORY_OPEN;
    }

    memset(&translated_tablename, 0, sizeof(int64_t));
    memcpy(&translated_tablename, tablename, strlen(tablename));

    v = btree_open(&table_index, db->fp, db->block_size, db->max_depth,
                   0, db->block_size);

    block.block_size = db->block_size;
    block.n_occupied = 0;
    block.block = malloc(db->block_size * sizeof(char));
    if (block.block == NULL) {
        r = 1;
        goto ERR_BLOCK_MALLOC;
    }


    ERR_INSERT:

        free(block.block);
    ERR_BLOCK_MALLOC:

        v = btree_close(&table_index);
        if (v != 0) {
            r = 4;
        }
    ERR_TABLE_INDEX_OPEN:

        v = btree_close(&directory);
        if (v != 0) {
            r = 4;
        }
    ERR_DIRECTORY_OPEN:

    return r;

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
        Closes the database.
        This function will only close db->fp.

        Return value:
            return value of fclose
    */

    return fclose(db->fp);

}
