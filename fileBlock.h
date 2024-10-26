#ifndef FILEBLOCK_H
#define FILEBLOCK_H

#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 64 // 256* 1024

typedef struct
{
    char data[BLOCK_SIZE];
    int next;
} FileBlock;

FileBlock *newFileBlock();
void setNameFileBlock(FileBlock *fileBlock, const char data[BLOCK_SIZE]);
void setNextFileBlock(FileBlock *fileBlock, int next);
void serializeFileBlock(FileBlock *fileBlock, FILE *file);
FileBlock *deserializeFileBlock(FILE *file);

#endif // FILEBLOCK_H