#ifndef FILEBLOCK_H
#define FILEBLOCK_H

#include <stddef.h> // Include this header for size_t
#include <stdio.h>  // Include this header for FILE
#include <unistd.h> // Include this header for ssize_t

#define BLOCK_SIZE 256 * 1024 // 256 KB

typedef struct FileBlock
{
    char data[BLOCK_SIZE];
    size_t size;
    struct FileBlock *next;
} FileBlock;

struct FileBlock *newFileBlock();
void setFileBlockData(struct FileBlock *fileBlock, char data[BLOCK_SIZE], ssize_t bytesRead);
void serializeFileBlock(struct FileBlock *fileBlock, FILE *file);
FileBlock *deserializeFileBlock(FILE *file);

#endif // FILEBLOCK_H