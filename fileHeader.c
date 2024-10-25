#include <stdio.h>
#include <string.h>

#include "fileHeader.h"
#include "verbose.h"

FileHeader *newFileHeader()
{
    FileHeader *fileHeader = (FileHeader *)malloc(sizeof(FileHeader));
    fileHeader->first = -1; // -1 means that the file has no blocks
    fileHeader->isDeleted = 0;
    return fileHeader;
}

void setNameFileHeader(FileHeader *fileHeader, char *name)
{
    strcpy(fileHeader->name, name);
}

void serializeFileHeader(FileHeader *fileHeader, FILE *file)
{
    logInfo("size of fileHeader: %ld\n", sizeof(FileHeader));
    fwrite(fileHeader, sizeof(FileHeader), 1, file);
}

FileHeader *deserializeFileHeader(FILE *file)
{
    FileHeader *fileHeader = newFileHeader();
    fread(fileHeader, sizeof(FileHeader), 1, file);
    logInfo(toStringFileHeader(fileHeader));
    return fileHeader;
}

char *toStringFileHeader(FileHeader *fileHeader)
{
    // name + size of int as string + extra chars in str
    char *str = (char *)malloc(sizeof(char) * FILE_NAME_SIZE + 10);
    sprintf(str, "FileHeader: name: %s, first: %d\n", fileHeader->name, fileHeader->first);
    return str;
}