#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fileBlock.h"

// File is a struct that represents a file divided by blocks of 512kb, it is a linked list
typedef struct File
{
    char *name;
    struct FileBlock *head;
    struct FileBlock *tail;

    int isDeleted;
} File;

// newFile creates a new File struct and opens the file with the given name
File *newFile();
void setNameFile(File *file, char *name);
void addBlock(File *file, struct FileBlock *block);
void serializeFile(struct File *myFile, FILE *file);
File *deserializeFile(FILE *myFile);

// used for instance of freeBlocks
struct FileBlock *getFreeBlock(File *file);

#endif // FILE_H