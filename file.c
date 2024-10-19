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
        printf("Error: opening source file.\n", file->name);
        return;
    }

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
            printf("Error: reading source file.\n", file->name);
            close(sourceFD);
            break;
        }
        
        // TODO : creat a new block and add it to the file
    }
}

FileBlock *newFileBlock()
{
    FileBlock *fileBlock = (FileBlock *)malloc(sizeof(FileBlock));
    if (fileBlock == NULL)
    {
        return NULL;
    }

    fileBlock->next = NULL;
    fileBlock->index = 0;

    return fileBlock;
}

void openFil