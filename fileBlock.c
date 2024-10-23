#include <stdlib.h> // Include this header for malloc and NULL
#include <string.h>
#include <stddef.h> // Include this header for size_t
#include <stdio.h>  // Include this header for FILE
#include <unistd.h> // Include this header for ssize_t

#include "fileBlock.h"
#include "verbose.h"
#include <cstddef>

struct FileBlock *newFileBlock()
{
    FileBlock *fileBlock = (FileBlock *)malloc(sizeof(FileBlock));

    fileBlock->size = 0;
    fileBlock->next = NULL;

    return fileBlock;
}

void setFileBlockData(struct FileBlock *fileBlock, char data[BLOCK_SIZE], ssize_t bytesRead)
{
    if (fileBlock == NULL)
    {
        logError("Error: FileBlock is null for setFileBlockData\n");
        return;
    }

    if (data == NULL)
    {
        logError("Error: Data is null for setFileBlockData\n");
        return;
    }

    if (bytesRead < 0)
    {
        logError("Error: bytesRead is negative for setFileBlockData\n");
        return;
    }

    memcpy(fileBlock->data, data, bytesRead);
    fileBlock->size = bytesRead;
}

void serializeFileBlock(FileBlock *fileBlock, FILE *file)
{
    fwrite(&(fileBlock->size), sizeof(size_t), 1, file);
    fwrite(fileBlock->data, sizeof(char), fileBlock->size, file);
}

struct FileBlock *deserializeFileBlock(FILE *file)
{
    FileBlock *fileBlock = newFileBlock();

    size_t bytesRead = fread(&(fileBlock->size), sizeof(size_t), 1, file);
    if (bytesRead != 1)
    {
        logError("Error: Could not read size from file\n");
        return NULL;
    }

    bytesRead = fread(fileBlock->data, sizeof(char), fileBlock->size, file);
    if (bytesRead != fileBlock->size)
    {
        logError("Error: Could not read data from file\n");
        return NULL;
    }

    return fileBlock;
}