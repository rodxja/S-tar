#include "file.h"

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