#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 256 * 1024 // 256 KB

// File is a struct that represents a file divided by blocks of 512kb, it is a linked list
typedef struct File
{
    char *name;
    struct FileBlock *Head;
    struct FileBlock *Tail;
} File;

File *newFile(char *name);

typedef struct
{
    char data[BLOCK_SIZE]; // Pointer to the block of data
    // size_t size;  // Size of the data in this block (could be less than 256 KB for the last block)
    size_t index; // Index of the block in the sequence of blocks (optional)
    struct FileBlock *next;
} FileBlock;

FileBlock *newFileBlock();

#endif // FILE_H