#include <fcntl.h>
#include <string.h>

#include "tableFile.h"
#include "verbose.h"

TableFile *newTableFile()
{
    TableFile *tableFile = (TableFile *)malloc(sizeof(TableFile));
    if (tableFile == NULL)
    {
        return NULL;
    }
    for (int i = 0; i < FILES_NUM; i++)
    {
        tableFile->files[i] = NULL;
    }
    tableFile->freeBlocks = NULL;
    tableFile->filesCount = 0;

    return tableFile;
}

void writeTableFile(TableFile *tableFile, char *outputFile)
{
    // no idea was b for
    // FILE *file = fopen(outputFile, "wb");
    FILE *file = fopen(outputFile, "w");
    if (file == NULL)
    {
        logError("Error: No se pudo abrir el archivo %s\n", outputFile);
        return;
    }

    int result = fwrite(tableFile, sizeof(TableFile), 1, file);
    if (result == 0)
    {
        logError("Error writing to the given file\n");
        exit(1);
    }
    else
    {
        logError("TableFile is written to the file successfully!\n");
    }

    fclose(file);
}

void addFile(TableFile *tableFile, char *name)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for addFile\n");
        return;
    }

    if (tableFile->filesCount == FILES_NUM)
    {
        logError("Error: TableFile is full\n");
        return;
    }

    if (name == NULL)
    {
        logError("Error: File name is null for addFile\n");
        return;
    }

    int sourceFD = open(name, O_RDONLY);
    if (sourceFD == -1)
    {
        logError("Error: opening source file '%s'.\n", name);
        return;
    }

    // ??? WOULD IT BE CORRECT TO USE A BUFFER OF 256KB?
    char buffer[BLOCK_SIZE];

    File *file = getFileToUse(tableFile);
    setNameFile(file, name);
    while (1)
    {
        ssize_t bytesRead = read(sourceFD, buffer, sizeof(buffer));
        if (bytesRead == 0)
        { // End of file
            break;
        }
        if (bytesRead == -1)
        {
            logError("Error: reading source file '%s'.\n", name);
            close(sourceFD);
            break;
        }

        struct FileBlock *fileBlock = getFileBlockToUse(tableFile);
        setFileBlockData(fileBlock, buffer, bytesRead);
        addBlock(file, fileBlock);
    }

    // TODO : return error code
    tableFile->files[tableFile->filesCount] = file;
    tableFile->filesCount++;
    close(sourceFD);
}

// this retrieves the file to use
File *getFileToUse(TableFile *tableFile)
{
    // TODO : implement a way to detect deleted files
    return newFile(NULL);
}

// this retrieves the file block to use
// it is taken from the free blocks list if there is any
// otherwise a new block is created
struct FileBlock *getFileBlockToUse(TableFile *tableFile)
{
    struct FileBlock *fileBlock = getFreeBlock(tableFile->freeBlocks);
    if (fileBlock != NULL)
    {
        return fileBlock;
    }
    return newFileBlock();
}

int fileExists(TableFile *tableFile, char *fileName)
{
    for (int i = 0; i < tableFile->filesCount; i++) // this will not be correct due that there will be deleted files, iterate with while or some other way
    {
        if (tableFile->files[i] == NULL)
        {
            continue;
        }
        if (tableFile->files[i]->name == NULL)
        {
            continue;
        }
        if (strcmp(tableFile->files[i]->name, fileName) == 0)
        {
            return 1;
        }
    }
    return 0;
}