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

    return tableFile;
}

// adds a new file into file headers and stored in disk directly
// TODO : adjust to new logic
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

    //Cambio de open a fopen
    int sourceFD = fopen(name, "rb");
    if (sourceFD == NULL)
    {
        logError("Error: opening source file '%s'.\n", name);
        return;
    }

    // ??? WOULD IT BE CORRECT TO USE A BUFFER OF 256KB?
    char buffer[BLOCK_SIZE];

    File *file = getFileToUse(tableFile);
    setNameFile(file, name);
    size_t offset = 0;
    while (1)
    {
        // Mover el puntero de lectura al inicio del bloque
        if (fseek(sourceFD, offset, SEEK_SET) != 0) { 
            logError("Error: seeking in source file '%s'.\n", name);
            fclose(sourceFD);
            return;
        }

        // Leer el bloque de datos
        size_t bytesRead = fread(buffer, 1, BLOCK_SIZE, sourceFD);  
        if (bytesRead == 0) {  // Fin del archivo
            break;
        }
        if (ferror(sourceFD)) {
            logError("Error: reading source file '%s'.\n", name);
            fclose(sourceFD);
            return;
        }

        FileBlock *fileBlock = getFileBlockToUse(tableFile);
        setFileBlockData(fileBlock, buffer, bytesRead);
        addBlock(file, fileBlock);

        offset += bytesRead;
    }
    // TODO : return error code
    tableFile->filesCount++;
    close(sourceFD);
}

// this retrieves the file to use, searches it there is one deleted, otherwise creates a new one
// !!! i think i will deprecate this function
File *getFileToUse(TableFile *tableFile)
{
    // TODO : implement a way to detect deleted files
    return newFile(NULL);
}

// this retrieves the file block to use
// it is taken from the free blocks list if there is any
// otherwise a new block is created
// TODO : adjust to new logic
struct FileBlock *getFileBlockToUse(TableFile *tableFile)
{
    /* struct FileBlock *fileBlock = getFreeBlock(tableFile->freeBlocks);
    if (fileBlock != NULL)
    {
        return fileBlock;
    } */
    return newFileBlock();
}

int fileExists(TableFile *tableFile, char *fileName)
{
    /* for (int i = 0; i < tableFile->filesCount; i++) // this will not be correct due that there will be deleted files, iterate with while or some other way
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
    } */
    return 0;
}

// TODO : adjust to new logic, load only the file headers
TableFile *loadTableFile(char *inputFile)
{
    TableFile *tableFile = deserializeTableFile(inputFile);
    return tableFile;
}

void extractFile(TableFile *tableFile, char *outputDirectory)
{
    // extract all valid files
    /* for (int i = 0; i < tableFile->filesCount; i++)
    {
        File *file = tableFile->files[i];
        if (file == NULL || file->name == NULL || file->head == NULL || file->isDeleted)
        {
            continue;
        }

        char outputPath[256];
        sprintf(outputPath, "%s/%s", outputDirectory, file->name);

        FILE *outputFile = fopen(outputPath, "w");
        if (outputFile == NULL)
        {
            logError("Error: No se pudo abrir el archivo %s\n", outputPath);
            return;
        }

        FileBlock *currentBlock = file->head;
        while (currentBlock != NULL)
        {
            fwrite(currentBlock->data, currentBlock->size, 1, outputFile);
            currentBlock = currentBlock->next;
        }

        fclose(outputFile);
        logInfo("El archivo %s ha sido extraído con éxito!\n", file->name);
    } */
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

    return tableFile;
}

void create(TableFile *tableFile, const char *outputFile)
{
    serializeTableFile(tableFile, outputFile);
}