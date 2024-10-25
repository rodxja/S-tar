#ifndef FILEHEADER_H
#define FILEHEADER_H

#include <stdlib.h>
#include <stdio.h>

#define FILE_NAME_SIZE 100

typedef struct
{
    char name[FILE_NAME_SIZE];
    int first;
} FileHeader;

FileHeader *newFileHeader();
void setNameFileHeader(FileHeader *fileHeader, char *name);
void serializeFileHeader(FileHeader *fileHeader, FILE *file);
FileHeader *deserializeFileHeader(FILE *file);

#endif // FILEHEADER_H
