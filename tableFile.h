#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <stdlib.h>

#include "fileHeader.h"
#include "fileBlock.h"

#define FILES_NUM 250

typedef struct
{
    // fileName is the name of the file .star
    char fileName[FILE_NAME_SIZE];
    // filesCount is the number of active files in the table, it can increase or decrease
    int filesCount;
    // blockCount is the number of blocks in the table
    // almost all the time it will increase
    // the only time it will decrease is when pack(-p) command is called
    int blockCount;
    // fileHeader is an array of pointers to FileHeader structs
    FileHeader *fileHeader[FILES_NUM];
    // freeBlocksHeader is a pointer to a FileHeader struct that represents the free blocks in the table
    FileHeader *freeBlocksHeader;

    // add FILE field in order to keep open the file
    FILE *file; // this will never be serialized or deserialized
} TableFile;

TableFile *newTableFile(const char *fileName);
void openTableFile(TableFile *tableFile, const char *mode);
void closeTableFile(TableFile *tableFile);
// adds a file to the TableFile, if the TableFile is full, it will print an error message
void addFile(TableFile *tableFile, const char *fileName);
int getOffsetTableFile();

// writes the TableFile to a file
FileHeader *getFileHeaderToUse(TableFile *tableFile);

int getBlockAvailable(TableFile *tableFile);

// gets a FileHeader from the TableFile by its name
FileHeader *getFileHeader(TableFile *tableFile, const char *fileName);
// loads a TableFile from a file
TableFile *loadTableFile(char *inputFile);

// extracts all files from the TableFile to a directory
void extractAllFiles(TableFile *tableFile, const char *outputDirectory);

void serializeTableFile(TableFile *table, const char *filename);
TableFile *deserializeTableFile(const char *filename);

// new usage
void create(TableFile *tableFile, const char *outputFile);
void delete(TableFile *tableFile, const char *fileName);
void update(TableFile *tableFile, const char *fileName);
void pack(TableFile *tableFile);

// list files
void listFiles(TableFile *tableFile);

int *getBlockList(FileHeader *fileHeader, FILE *file);
int *orderedBlockList(FileHeader *fileHeader, FILE *file);
void writeOrderedBlockList(FileHeader *fileHeader, FILE *file);

char *toStringTableFile(TableFile *tableFile);

#endif // TABLEINFO_H
