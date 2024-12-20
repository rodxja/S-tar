#include <stdio.h>
#include <string.h>

#include "fileHeader.h"
#include "verbose.h"

FileHeader *newFileHeader()
{
    FileHeader *fileHeader = (FileHeader *)malloc(sizeof(FileHeader));
    fileHeader->first = -1; // -1 means that the file has no blocks
    fileHeader->last = -1;
    fileHeader->isDeleted = 0;
    fileHeader->size = 0;
    fileHeader->index = -1;
    fileHeader->totalBlocks = 0;
    return fileHeader;
}

void setNameFileHeader(FileHeader *fileHeader, const char *name)
{
    strcpy(fileHeader->name, name);
}

void serializeFileHeader(FileHeader *fileHeader, FILE *file)
{
    fwrite(fileHeader, sizeof(FileHeader), 1, file);
}

FileHeader *deserializeFileHeader(FILE *file)
{
    FileHeader *fileHeader = newFileHeader();
    fread(fileHeader, sizeof(FileHeader), 1, file);
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

void printFileHeader(FileHeader *fileHeader, int *blockList)
{
    logInfo(toStringFileHeader(fileHeader));

    logInfo("Blocks : ");
    if (blockList == NULL)
    {
        logInfo("no blocks\n");
        return;
    }

    for (int j = 0; j < fileHeader->totalBlocks; j++)
    {
        if (j == fileHeader->totalBlocks - 1)
        {
            logInfo("%d", blockList[j]);
        }
        else
        {
            logInfo("%d -> ", blockList[j]);
        }
    }
    logInfo("\n");
}

int getOffsetFileHeader()
{
    return sizeof(FileHeader); // sizeof(char[FILE_NAME_SIZE]) + 6 * sizeof(int);
}

void resetFileHeader(FileHeader *fileHeader)
{
    fileHeader->first = -1;
    fileHeader->last = -1;
    fileHeader->isDeleted = 0;
    fileHeader->size = 0;
    fileHeader->index = -1;
    fileHeader->totalBlocks = 0;
}