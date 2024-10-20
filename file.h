#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BLOCK_SIZE 256 * 1024 // 256 KB

typedef struct FileBlock
{
    char data[BLOCK_SIZE];
    size_t size;
    struct FileBlock *next;
} FileBlock;

struct FileBlock *newFileBlock();
void setFileBlockData(struct FileBlock *fileBlock, char data[BLOCK_SIZE], ssize_t bytesRead);

// File is a struct that represents a file divided by blocks of 512kb, it is a linked list
typedef struct File
{
    char *name;
    struct FileBlock *head;
    struct FileBlock *tail;
} File;

// newFile creates a new File struct and opens the file with the given name
File *newFile();
void setNameFile(File *file, char *name);
void addBlock(File *file, struct FileBlock *block);

// used for list of freeBlocks
struct FileBlock *getFreeBlock(File *file);

#endif // FILE_H