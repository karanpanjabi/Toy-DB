#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "block.h"


int block_read(Block *s, FILE *fp, int64_t offset)
{

    /*
        s: Pointer to an allocated Block where the read data will be
           stored. Block size will be taken from s->block_size
        fp: File pointer to file from where block must be read
        offset: Offset in the file from where the block should be read
    */

    

}


int block_write(Block *s, FILE *fp, int64_t offset)
{

    

}


int block_append(Block *s, FILE *fp)
{

    

}
