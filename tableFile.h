#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <stdlib.h>
#include "file.h"
#include "fileHeader.h"

#define FILES_NUM 250

typedef struct
{
    int filesCount;
    FileHeader *fileHeader[FILES_NUM];
    FileHeader *freeBlocksHeader;
} TableFile;

// creates a new TableFile struct
TableFile *newTableFile();
// adds a file to the TableFile, if the TableFile is full, it will print an error message
void addFile(TableFile *tableFile, char *name);
// writes the TableFile to a file
File *getFileToUse(TableFile *tableFile);
struct FileBlock *getFileBlockToUse(TableFile *tableFile);

// checks if a file exists in the TableFile
int fileExists(TableFile *tableFile, char *fileName);
// loads a TableFile from a file
TableFile *loadTableFile(char *inputFile);

// extracts all files from the TableFile to a directory
void extractFile(TableFile *tableFile, char *outputDirectory);

void serializeTableFile(TableFile *table, const char *filename);
TableFile *deserializeTableFile(const char *filename);

// new usage
void create(TableFile *tableFile, const char *outputFile);

#endif // TABLEINFO_H
