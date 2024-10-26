#include <stdio.h>
#include <string.h>

#include "fileHeader.h"
#include "verbose.h"

FileHeader *newFileHeader()
{
    FileHeader *fileHeader = (FileHeader *)malloc(sizeof(FileHeader));
    fileHeader->first = -1; // -1 means that the file has no blocks
    fileHeader->isDeleted = 0;
    fileHeader->size = 0;
    fileHeader->index = -1;
    return fileHeader;
}

void setNameFileHeader(FileHeader *fileHeader, const char *name)
{
    strcpy(fileHeader->name, name);
}

void serializeFileHeader(FileHeader *fileHeader, FILE *file)
{
    logDebug("serialize %s", toStringFileHeader(fileHeader));
    fwrite(fileHeader, sizeof(FileHeader), 1, file);
}

FileHeader *deserializeFileHeader(FILE *file)
{
    FileHeader *fileHeader = newFileHeader();
    fread(fileHeader, sizeof(FileHeader), 1, file);
    logDebug("deserialize %s", toStringFileHeader(fileHeader));
    return fileHeader;
}

char *toStringFileHeader(FileHeader *fileHeader)
{
    // name + size of int as string + extra chars in str
    char *str = (char *)malloc(sizeof(char) * FILE_NAME_SIZE + 10);
    sprintf(str, "FileHeader: name: '%s', first: %d, last: %d, isDeleted: %d, size: %d, index: %d, blocks: %d.\n", fileHeader->name, fileHeader->first, fileHeader->last, fileHeader->isDeleted, fileHeader->size, fileHeader->index, fileHeader->totalBlocks);
    return str;
}

int isFileHeaderAvailable(FileHeader *fileHeader)
{
    return fileHeader->first == -1 || fileHeader->isDeleted;
}