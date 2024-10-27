#include <string.h>

#include "fileBlock.h"
#include "verbose.h"

FileBlock *newFileBlock()
{
    FileBlock *fileBlock = (FileBlock *)malloc(sizeof(FileBlock));
    if (fileBlock == NULL)
    {
        return NULL;
    }

    fileBlock->next = -1;
    return fileBlock;
}

void setNameFileBlock(FileBlock *fileBlock, const char data[BLOCK_SIZE])
{
    strcpy(fileBlock->data, data);
}

void setNextFileBlock(FileBlock *fileBlock, int next)
{
    fileBlock->next = next;
}

void serializeFileBlock(FileBlock *fileBlock, FILE *file)
{
    size_t bytesWritten = fwrite(fileBlock, sizeof(FileBlock), 1, file);
    if (bytesWritten < 1)
    {
        logError("Error: writing file block\n");
    }
}

FileBlock *deserializeFileBlock(FILE *file)
{
    FileBlock *fileBlock = newFileBlock();
    if (fileBlock == NULL)
    {
        return NULL;
    }

    size_t bytesRead = fread(fileBlock, sizeof(FileBlock), 1, file);
    if (bytesRead < 1)
    {
        logError("Error: reading file block\n");
        return NULL;
    }

    return fileBlock;
}

int getOffsetFileBlock()
{
    return sizeof(FileBlock);
}