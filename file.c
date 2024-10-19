#include <fcntl.h>

#include "file.h"
#include <stdio.h>
#include <unistd.h>

File *newFile(char *name)
{
    File *file = (File *)malloc(sizeof(File));
    if (file == NULL)
    {
        return NULL;
    }

    file->name = name;
    file->Head = NULL;
    file->Tail = NULL;

    return file;
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

    if (file->Head == NULL)
    {
        file->Head = block;
        file->Tail = block;
    }
    else
    {
        file->Tail->next = block;
        file->Tail = block;
    }
}

struct FileBlock *newFileBlock(char data[BLOCK_SIZE], ssize_t bytesRead)
{
    FileBlock *fileBlock = (FileBlock *)malloc(sizeof(FileBlock));
    if (fileBlock == NULL)
    {
        return NULL;
    }

    // Copy data into fileBlock->data
    memcpy(fileBlock->data, data, BLOCK_SIZE);
    fileBlock->size = bytesRead;
    fileBlock->next = NULL;
    // fileBlock->index = 0;

    return fileBlock;
}
