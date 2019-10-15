#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "block.h"


int block_read(Block *s, FILE *fp, int64_t offset)
{

    /*
        s: Pointer to an allocated Block where the read data will be
           stored. Number of bytes to be write will be taken from
           s->n_occupied.  s->block must be allocated by the caller.
        fp: File pointer to file from where block must be read. File
            must be opened in a mode where read is permitted.
        offset: Offset from the beginning the file from where the
                block should be read

        Return Value:
            0: Success
            1: Error or End-of-file. Caller should call ferror and
               feof to determine which occurred.
            2: fseek error. errno is set by fseek.
    */

    if (fseek(fp, offset, SEEK_SET) == -1) {
        return 2;
    }

    if (fread(s->block, sizeof(char), s->n_occupied, fp)
            < s->n_occupied) {
        return 1;
    }

    return 0;

}


int block_write(Block *s, FILE *fp, int64_t offset)
{

    /*
        s: Pointer to an allocated Block where data will be written.
           Data to write will be taken from s->block and
           s->n_occupied bytes will be written if call is successful.
        fp: File pointer to file where block must be written. File
            must be opened in a mode where write to arbitrary offset
            is permitted.
        offset: Offset from the beginning the file from where the
                block should be written.

        Return Value:
            0: Success
            1: Write error
            2: fseek error. errno is set by fseek.
    */

    if (fseek(fp, offset, SEEK_SET) == -1) {
        return 2;
    }

    if (fwrite(s->block, sizeof(char), s->n_occupied, fp)
            < s->n_occupied) {
        return 1;
    }

    return 0;

}


int block_append(Block *s, FILE *fp)
{

    /*
        s: Pointer to an allocated Block where data will be appended.
           Data to append will be taken from s->block and
           s->n_occupied bytes will be appended if call is successful.
        fp: File pointer to file where block must be appended. File
            must be opened in a mode where append is permitted.

        Return Value:
            0: Success
            1: Write error
            2: fseek error. errno is set by fseek.
    */

}
