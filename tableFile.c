#include <fcntl.h>
#include <string.h>

#include "tableFile.h"
#include "fileHeader.h"
#include "verbose.h"

// creates a new TableFile struct with all file headers initialized as empty
TableFile *newTableFile()
{
    TableFile *tableFile = (TableFile *)malloc(sizeof(TableFile));
    if (tableFile == NULL)
    {
        return NULL;
    }
    for (int i = 0; i < FILES_NUM; i++)
    {
        // instanciate all file headers in order to save them at the beginnig of file
        tableFile->fileHeader[i] = newFileHeader();
    }

    tableFile->freeBlocksHeader = newFileHeader();
    setNameFileHeader(tableFile->freeBlocksHeader, "freeBlocksHeader");

    tableFile->filesCount = 0;
    tableFile->blockCount = 0;

    return tableFile;
}

// adds a new file into file headers and stored in disk directly
// TODO : adjust to new logic
void addFile(TableFile *tableFile, const char *fileName)
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

    if (fileName == NULL)
    {
        logError("Error: File name is null for addFile\n");
        return;
    }

    int sourceFD = open(fileName, O_RDONLY);
    if (sourceFD == -1)
    {
        logError("Error: opening source file '%s'.\n", fileName);
        return;
    }

    FileHeader *fileHeader = getFileToUse(tableFile);
    if (fileHeader == NULL)
    {
        logError("Error: no file header available\n");
        close(sourceFD);
        return;
    }
    setNameFileHeader(fileHeader, fileName);

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
            logError("Error: reading source file '%s'.\n", fileName);
            close(sourceFD);
            break;
        }

        // ??? what would happen if a block is exactly 256KB?

        // take first block and set into the fileHeader
        int firstBlock = tableFile->freeBlocksHeader->first;
        if (firstBlock == -1)
        {
            firstBlock = tableFile->blockCount;
            tableFile->blockCount++;
        }

        with 

        // current block must point to the next block

        struct FileBlock *fileBlock = getFileBlockToUse(tableFile);
        setFileBlockData(fileBlock, buffer, bytesRead);
        addBlock(file, fileBlock);
    }

    // TODO : return error code
    tableFile->filesCount++;
    close(sourceFD);
}

// this retrieves the file header to use, searches it there is one deleted, otherwise creates a new one
FileHeader *getFileHeaderToUse(TableFile *tableFile)
{
    int filesToSearch = tableFile->filesCount;

    if (filesToSearch == 0)
    {
        return 0;
    }

    for (int i = 0; i < FILES_NUM; i++)
    {
        if (filesToSearch == 0)
        {
            break;
        }

        if (tableFile->fileHeader[i] == NULL)
        {
            continue;
        }

        if (isFileHeaderAvailable(tableFile->fileHeader[i]))
        {
            return tableFile->fileHeader[i];
        }

        filesToSearch--;
    }
    return NULL;
}

int fileExists(TableFile *tableFile, const char *fileName)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for fileExists\n");
        return 0;
    }

    int filesToSearch = tableFile->filesCount;

    if (filesToSearch == 0)
    {
        return 0;
    }

    for (int i = 0; i < FILES_NUM; i++) // this will not be correct due that there will be deleted files, iterate with while or some other way
    {
        if (filesToSearch == 0)
        {
            break;
        }

        if (tableFile->fileHeader[i] == NULL)
        {
            continue;
        }

        if (tableFile->fileHeader[i]->isDeleted)
        {
            continue;
        }

        if (strcmp(tableFile->fileHeader[i]->name, fileName) == 0)
        {
            return 1;
        }

        filesToSearch--;
    }
    return 0;
}

// TODO : adjust to new logic, load only the file headers
TableFile *loadTableFile(char *inputFile)
{
    TableFile *tableFile = deserializeTableFile(inputFile);
    return tableFile;
}

// TODO : add the file name of the .star file
void extractAllFiles(TableFile *tableFile, const char *outputDirectory)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for extractAllFiles\n");
        return;
    }

    if (outputDirectory == NULL)
    {
        logError("Error: outputDirectory is null for extractAllFiles\n");
        return;
    }

    if (tableFile->filesCount == 0)
    {
        logInfo("Info: No files to extract\n");
        return;
    }

    // number of files to extract
    int filesToExtract = tableFile->filesCount;
    // extract all valid files
    for (int i = 0; i < tableFile->filesCount; i++)
    {
        if (filesToExtract == 0)
        {
            break;
        }

        FileHeader *fileHeader = tableFile->fileHeader[i];
        if (fileHeader == NULL)
        {
            logError("Error: fileHeader is null in extractAllFiles\n");
        }

        // check if the file is deleted or has no blocks
        if (fileHeader->isDeleted || fileHeader->first == -1)
        {
            continue;
        }

        char outputPath[256];
        sprintf(outputPath, "%s/%s", outputDirectory, fileHeader->name);
        logInfo("Extracting file %s to %s\n", fileHeader->name, outputPath);

        FILE *outputFile = fopen(outputPath, "w");
        if (outputFile == NULL)
        {
            logError("Error: No se pudo abrir el archivo %s\n", outputPath);
            return;
        }

        // by the first block start to read from memory

        // TODO : use seek to move to the first block

        FileBlock *currentBlock = file->head;
        while (currentBlock != NULL)
        {
            fwrite(currentBlock->data, currentBlock->size, 1, outputFile);
            currentBlock = currentBlock->next;
        }

        fclose(outputFile);
        filesToExtract--;
        logInfo("El archivo %s ha sido extraído con éxito!\n", file->name);
    }
}

// Function to serialize the TableFile structure
void serializeTableFile(TableFile *tableFile, const char *outputFile)
{
    FILE *file = fopen(outputFile, "wb");
    if (!file)
    {
        logError("Failed to open file for writing");
        return;
    }

    // serialize file headers
    for (int i = 0; i < FILES_NUM; i++)
    {
        serializeFileHeader(tableFile->fileHeader[i], file);
    }

    // serialize free blocks header
    serializeFileHeader(tableFile->freeBlocksHeader, file);

    fwrite(&tableFile->filesCount, sizeof(tableFile->filesCount), 1, file);
    fwrite(&tableFile->blockCount, sizeof(tableFile->blockCount), 1, file);
    fclose(file);
}

// Function to deserialize the TableFile structure
TableFile *deserializeTableFile(const char *filename)
{
    TableFile *tableFile = newTableFile();
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        logError("Failed to open file for reading");
        return NULL;
    }

    // validate

    // deserialize file headers
    for (int i = 0; i < FILES_NUM; i++)
    {
        tableFile->fileHeader[i] = deserializeFileHeader(file);
    }

    // deserialize free blocks header
    tableFile->freeBlocksHeader = deserializeFileHeader(file);

    fread(&tableFile->filesCount, sizeof(tableFile->filesCount), 1, file);
    fread(&tableFile->blockCount, sizeof(tableFile->blockCount), 1, file);

    fclose(file);

    return tableFile;
}

void create(TableFile *tableFile, const char *outputFile)
{
    serializeTableFile(tableFile, outputFile);
}