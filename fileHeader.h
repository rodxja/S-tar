#ifndef FILEHEADER_H
#define FILEHEADER_H

#include <stdlib.h>
#include <stdio.h>

#define FILE_NAME_SIZE 100

typedef struct
{
    char name[FILE_NAME_SIZE];
    int first;
    int last;
    int isDeleted;
    int size;
    int index;
    int totalBlocks;
} FileHeader;

FileHeader *newFileHeader();
void setNameFileHeader(FileHeader *fileHeader, const char *name);
void serializeFileHeader(FileHeader *fileHeader, FILE *file);
FileHeader *deserializeFileHeader(FILE *file);
char *toStringFileHeader(FileHeader *fileHeader);
int isFileHeaderAvailable(FileHeader *fileHeader);
void printFileHeader(FileHeader *fileHeader, int *blockList);
int getOffsetFileHeader();
void resetFileHeader(FileHeader *fileHeader);

#endif // FILEHEADER_H
