#ifndef FILE_H
#define FILE_H


#include <inttypes.h>
#include <stdio.h>


typedef struct Block {

    int64_t block_size;
    int64_t n_occupied;
    char *block;

} Block;

int block_read(Block *s, FILE *fp, int64_t offset);
int block_write(Block *s, FILE *fp, int64_t offset);
int block_append(Block *s, FILE *fp);


#endif
