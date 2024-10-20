#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "file.h"

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
        printf("Error: File is null for setNameFile\n");
        return;
    }

    if (name == NULL)
    {
        printf("Error: Name is null for setNameFile\n");
        return;
    }

    file->name = name;
}

void openFile(File *file)
{
    if (file == NULL)
    {
        printf("Error: File is null for openFile\n");
        return;
    }

    if (file->name == NULL)
    {
        printf("Error: File name is null for openFile\n");
        return;
    }

    int sourceFD = open(file->name, O_RDONLY);
    if (sourceFD == -1)
    {
        printf("Error: opening source file '%s'.\n", file->name);
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
            printf("Error: reading source file '%s'.\n", file->name);
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
        printf("Error: File is null for addBlock\n");
        return;
    }

    if (block == NULL)
    {
        printf("Error: Block is null for addBlock\n");
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
        printf("Error: FileBlock is null for setFileBlockData\n");
        return;
    }

    if (data == NULL)
    {
        printf("Error: Data is null for setFileBlockData\n");
        return;
    }

    if (bytesRead < 0)
    {
        printf("Error: bytesRead is negative for setFileBlockData\n");
        return;
    }

    memcpy(fileBlock->data, data, bytesRead);
    fileBlock->size = bytesRead;
}

struct FileBlock *getFreeBlock(File *file)
{
    if (file == NULL)
    {
        printf("Error: File is null for getFreeBlock\n");
        return NULL;
    }

    if (file->head == NULL)
    {
        printf("Error: File has no blocks for getFreeBlock\n");
        return NULL;
    }

    struct FileBlock *freeBlock = file->head;
    file->head = file->head->next;

    return freeBlock;
}