#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "file.h"
#include "verbose.h"
#include "fileBlock.h"

File *newFile()
{
    File *file = (File *)malloc(sizeof(File));

    file->name = NULL;
    file->head = NULL;
    file->tail = NULL;
    file->isDeleted = 0;

    return file;
}

void setNameFile(File *file, char *name)
{
    if (file == NULL)
    {
        logError("Error: File is null for setNameFile\n");
        return;
    }

    if (name == NULL)
    {
        logError("Error: Name is null for setNameFile\n");
        return;
    }

    if (file->name != NULL)
    {
        free(file->name);
    }

    file->name = (char *)malloc(strlen(name) + 1);
    strcpy(file->name, name);
}

void addBlock(File *file, struct FileBlock *block)
{
    if (file == NULL)
    {
        logError("Error: File is null for addBlock\n");
        return;
    }

    if (block == NULL)
    {
        logError("Error: Block is null for addBlock\n");
        return;
    }

    if (file->head == NULL)
    {
        file->head = block;
        file->tail = block;
    }
    else
    {
        file->tail->next = block;
        file->tail = block;
    }
}

void serializeFile(struct File *myFile, FILE *file)
{
    size_t nameLen = strlen(myFile->name) + 1; // Include the null terminator
    // write size of name
    fwrite(&nameLen, sizeof(size_t), 1, file);
    // write name
    fwrite(myFile->name, sizeof(char), nameLen, file);
    // write blocks
    struct FileBlock *currentBlock = myFile->head;
    while (currentBlock)
    {
        serializeFileBlock(currentBlock, file);
        currentBlock = currentBlock->next;
    }
    // write null to indicate end of blocks
    FileBlock *nullBlock = newFileBlock();
    fwrite(nullBlock, sizeof(struct FileBlock *), 1, file);
    // write isDeleted
    fwrite(&(myFile->isDeleted), sizeof(int), 1, file);
}

File *deserializeFile(FILE *file)
{
    File *f = newFile();
    // read size of name
    size_t nameLen;
    fread(&nameLen, sizeof(size_t), 1, file);
    // read name
    f->name = (char *)malloc(nameLen);
    fread(f->name, sizeof(char), nameLen, file);
    // read blocks
    while (1)
    {
        struct FileBlock *nextBlock = deserializeFileBlock(file);
        // this is due to nullBlock that was serialized
        if (nextBlock->size == 0)
        {
            break;
        }
        addBlock(f, nextBlock);
    }

    // read isDeleted
    fread(&(f->isDeleted), sizeof(int), 1, file);

    return f;
}

struct FileBlock *getFreeBlock(File *file)
{
    if (file == NULL)
    {
        logError("Error: File is null for getFreeBlock\n");
        return NULL;
    }

    if (file->head == NULL)
    {
        logError("Error: File has no blocks for getFreeBlock\n");
        return NULL;
    }

    struct FileBlock *freeBlock = file->head;
    file->head = file->head->next;

    return freeBlock;
}