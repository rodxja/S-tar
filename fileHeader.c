#include <stdio.h>
#include <string.h>

#include "fileHeader.h"
#include "verbose.h"

FileHeader *newFileHeader()
{
    FileHeader *fileHeader = (FileHeader *)malloc(sizeof(FileHeader));
    fileHeader->first = 0;
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
    return fileHeader;
}