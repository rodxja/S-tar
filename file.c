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

    file->name = name;
}

void openFile(File *file)
{
    if (file == NULL)
    {
        logError("Error: File is null for openFile\n");
        return;
    }

    if (file->name == NULL)
    {
        logError("Error: File name is null for openFile\n");
        return;
    }

    int sourceFD = open(file->name, O_RDONLY);
    if (sourceFD == -1)
    {
        logError("Error: opening source file '%s'.\n", file->name);
        return;
    }

    // ??? WOULD IT BE CORRECT TO USE A BUFFER OF 256KB?
    char buffer[BLOCK_SIZE];

    while (1)
    {
        ssize_t bytesRead = read(sourceFD, buffer, sizeof(buffer));
        if (bytesRead == 0)
        { // End of file
            break;
        }
        if (bytesRead == -1)
        {
            logError("Error: reading source file '%s'.\n", file->name);
            close(sourceFD);
            break;
        }

        struct FileBlock *fileBlock = newFileBlock(buffer, bytesRead);
        addBlock(file, fileBlock);
    }
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

void serializeFileList(struct File *myFile, FILE *file)
{
    size_t nameLen = strlen(myFile->name) + 1; // Include the null terminator
    // write size of name
    fwrite(&nameLen, sizeof(size_t), 1, file);
    // write name
    fwrite(myFile->name, sizeof(char), nameLen, file);
    serializeFileBlockList(myFile->head, file);
}

File *deserializeFileList(FILE *file)
{
    File *f = (File *)malloc(sizeof(File));
    size_t nameLen;
    fread(&nameLen, sizeof(size_t), 1, file);
    f->name = (char *)malloc(nameLen);
    fread(f->name, sizeof(char), nameLen, file);
    f->head = deserializeFileBlockList(file);
    while (f->head->next)
    {
        f->head = f->head->next;
    }
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