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

// newFile creates a new File struct and opens the file with the given name
File *newFile(char *name);
void openFile(File *file);
// TODO : peding to implement
void addBlock(File *file, FileBlock *block);

typedef struct
{
    char data[BLOCK_SIZE]; // Pointer to the block of data
    // NULL in all cases where block is full
    // otherwise, it will specify 
    int *offset;
    struct FileBlock *next;
} FileBlock;

FileBlock *newFileBlock(char data[BLOCK_SIZE]);

#endif // FILE_H