#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <stdlib.h>
#include "file.h"

#define FILES_NUM 250

typedef struct
{
    int filesCount;
    File *files[FILES_NUM];
    File *freeBlocks;
} TableFile;

// creates a new TableFile struct
TableFile *newTableFile();
// adds a file to the TableFile, if the TableFile is full, it will print an error message
void addFile(TableFile *tableFile, char *name);
// writes the TableFile to a file
void writeTableFile(TableFile *tableFile, char *outputFile);
File *getFileToUse(TableFile *tableFile);
struct FileBlock *getFileBlockToUse(TableFile *tableFile);

int fileExists(TableFile *tableFile, char *fileName);
TableFile *loadTableFile(char *inputFile);
void extractFile(TableFile *tableFile, char *outputDirectory);

void serializeTableFile(TableFile *table, const char *filename);
TableFile *deserializeTableFile(const char *filename);

#endif // TABLEINFO_H
