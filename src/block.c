#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "block.h"


int block_read(Block *s, FILE *fp, int64_t offset)
{

    /*
        s: Pointer to an allocated Block where the read data will be
           stored. Block size will be taken from s->block_size.
           s->block must be allocated by the caller.
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

    if (fread(s->block, sizeof(char), s->block_size, 1, fp) == 0) {
        return 1;
    }

    return 0;

}


int block_write(Block *s, FILE *fp, int64_t offset)
{

    

}


int block_append(Block *s, FILE *fp)
{

    

}
