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
#include "util.h"

#define BLKROUNDDOWN(adr, blksize) ((adr / blksize) * blksize)
#define BLKROUNDUP_C(adr, blksize) (BLKROUNDDOWN(adr, blksize) + blksize)
#define BLKROUNDUP(adr, blksize) (adr % blksize != 0 ? BLKROUNDUP_C(adr, blksize) : adr)

#define MIN(a, b) ((b) < (a) ? (b) : (a))

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

    if ((fw_rv = fwrite(&block_size, sizeof(int32_t), 1, fp)) != 1) {
        r = 3;
        goto FWRITE_FAILED;
    }
    bytes_written += sizeof(int32_t);

    if ((fw_rv = fwrite(&max_depth, sizeof(int32_t), 1, fp)) != 1) {
        r = 3;
        goto FWRITE_FAILED;
    }
    bytes_written += sizeof(int32_t);

    char c = '\0';
    if ((fw_rv = fwrite(&c, sizeof(char),
                            (block_size - bytes_written), fp))
             < (block_size - bytes_written)) {
        r = 3;
        goto FWRITE_FAILED;
    }

    ret = btree_open(&directory, fp, block_size, max_depth, 1, 0);  // why 0 over here -> 0 is ignored because do_create = 1
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
                    Schema *schema)
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
    Btree directory, table_index;

    // generic failure return value
    r = 1;

    /* First schema element must be int64_t.
       It will be the primary key.
       If it is not an int64_t, return 2 to indicate this failure.  */
    if (schema->elements[0].dtype != 0) {
        r = 2;
        goto ERR_INVALID_SCHEMA;
    }

    translated_tablename = 0;
    memcpy(&translated_tablename, tablename, MIN(sizeof(int64_t), strlen(tablename)));  // make sure we copy only the amt translated_tablename can hold

    // printf("%s\n", translated_tablename);

    v = btree_open(&directory, db->fp, db->block_size, db->max_depth,
                   0, db->block_size);
    if (v != 0) {
        r = 4;
        goto ERR_DIRECTORY_OPEN;
    }

    fseek(db->fp, 0, SEEK_END);     // to figure out where the new table is inserted
    int64_t new_table_pos = BLKROUNDUP((int64_t)ftell(db->fp), db->block_size);
    btree_insert(&directory, translated_tablename, new_table_pos);    

    fseek(db->fp, new_table_pos, SEEK_SET);
    // TODO: print directory btree

    // insert schema at this position
    fwrite(schema, sizeof(Schema), 1, db->fp);

    // create btree for this table in the next block
    v = btree_open(&table_index, db->fp, db->block_size, db->max_depth, 1, 0);

    // TODO: verify if btree got created
    
    if (v != 0) {
        r = 4;
        goto ERR_BTREE_CREATE;
    }
    v = btree_close(&table_index);
    if (v != 0) {
        r = 4;
        goto ERR_BTREE_CREATE;
    }

    r = 0;

    ERR_CREATE:
    ERR_BTREE_CREATE:

    ERR_BLOCK_MALLOC:

        v = btree_close(&directory);
        if (v != 0) {
            r = 4;
        }
    ERR_DIRECTORY_OPEN:
    ERR_INVALID_SCHEMA:

    return r;

}


int db_insert(Database *db, char *tablename, DataRecord *records, int n_records)
{

    /*
        
    */

    int r, v;
    int i, j;
    int64_t index_offset;
    int64_t ret;
    int64_t translated_tablename;
    char *dtype;
    Btree directory, table_index;

    // generic failure return value
    r = 1;

    v = btree_open(&directory, db->fp, db->block_size, db->max_depth,
                   0, db->block_size);
    if (v != 0) {
        r = 4;
        goto ERR_DIRECTORY_OPEN;
    }

    // finding integer translation of table name
    // memset(&translated_tablename, 0, sizeof(int64_t));
    translated_tablename = 0;
    memcpy(&translated_tablename, tablename, 7);

    // finding offset of table index
    v = btree_search(&directory, translated_tablename, &index_offset);
    if (v != 0) {
        r = 2;
        goto ERR_INSERT;
    }

    // read schema from index_offset
    fseek(db->fp, index_offset, SEEK_SET);
    Schema schema;
    fread(&schema, sizeof(Schema), 1, db->fp);

    // read the btree for the table
    v = btree_open(&table_index, db->fp, db->block_size, db->max_depth, 0, index_offset + db->block_size );     // might have to use pgrounddown on the other side
    if (v != 0) {
        r = 4;
        goto ERR_TABLE_INDEX_OPEN;
    }

    // insert according to schema
    // verify schema and find size of data to be written
    int size = 0;
    if(n_records != schema.n)
    {
        return -1; // wrong number of columns
    }
    for (int i = 0; i < schema.n; i++)
    {
        if(schema.elements[i].dtype != records[i].dtype)
            return -1;  // wrong type
        
        if(schema.elements[i].dtype == 0)
            size += sizeof(int64_t);
        else if(schema.elements[i].dtype == 1)
            size += strlen(records[i].data) + 1;
        else if(schema.elements[i].dtype == 2)
            size += sizeof(float);
    }
    
    // seek to the end where the data has to be inserted
    fseek(db->fp, 0, SEEK_END);
    int64_t record_pos = ftell(db->fp);

    // check if btree already has that key
    v = btree_insert(&table_index, *(int64_t *) records[0].data, record_pos );
    if(v == -1)
    {
        return -1; // key already is present
    }
    
    // assign buffer to load data
    void *buffer = (void *) malloc(size * sizeof(void));
    int copied_size = 0;
    for (int i = 0; i < n_records; i++)
    {
        int vsize = 0;
        if(records[i].dtype == 0)
            vsize = sizeof(int64_t);
        else if(records[i].dtype == 1)
            vsize = strlen(records[i].data) + 1;
        else if(records[i].dtype == 2)
            vsize = sizeof(float);
        memcpy(buffer + copied_size, records[i].data, vsize);
        copied_size += vsize;
    }

    fseek(db->fp, record_pos, SEEK_SET);
    fwrite(buffer, sizeof(void), size, db->fp);
    
    free(buffer);

    r = 0;

    ERR_INSERT:

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
        if n_cols = 0, select *
    */

    int r, v;
    int i, j;
    int64_t index_offset;
    int64_t ret;
    int64_t translated_tablename;
    char *dtype;
    Btree directory, table_index;

    // generic failure return value
    r = 1;

    v = btree_open(&directory, db->fp, db->block_size, db->max_depth,
                   0, db->block_size);
    if (v != 0) {
        r = 4;
        return -1;
    }

    // finding integer translation of table name
    // memset(&translated_tablename, 0, sizeof(int64_t));
    translated_tablename = 0;
    memcpy(&translated_tablename, tablename, 7);

    // finding offset of table index
    v = btree_search(&directory, translated_tablename, &index_offset);
    if (v != 0) {
        r = 2;
        return -1;
    }

    // read schema from index_offset
    fseek(db->fp, index_offset, SEEK_SET);
    Schema schema;
    fread(&schema, sizeof(Schema), 1, db->fp);

    // read the btree for the table
    v = btree_open(&table_index, db->fp, db->block_size, db->max_depth, 0, index_offset + db->block_size );     // might have to use pgrounddown on the other side
    if (v != 0) {
        r = 4;
        return -1;
    }

    // TODO: verify the columns with the ones in schema

    // read the entire table
    // store the offsets to data in some list
    // fetch the data at the offsets and print according to schema
    // print/append to a struct those values which satisfy the condition

    recurse_tree_data(&table_index, &schema);
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
