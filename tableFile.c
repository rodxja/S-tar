#include <fcntl.h>
#include <string.h>

#include "tableFile.h"
#include "fileHeader.h"
#include "verbose.h"

// creates a new TableFile struct with all file headers initialized as empty
TableFile *newTableFile(const char *fileName)
{
    if (fileName == NULL)
    {
        logError("Error: File name is null for newTableFile\n");
        return NULL;
    }

    TableFile *tableFile = (TableFile *)malloc(sizeof(TableFile));
    if (tableFile == NULL)
    {
        return NULL;
    }

    strcpy(tableFile->fileName, fileName);

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

    FILE *sourceFD = fopen(fileName, O_RDONLY);
    if (!sourceFD)
    {
        logError("Error: opening source file '%s'.\n", fileName);
        return;
    }

    FileHeader *fileHeader = getFileHeaderToUse(tableFile);
    if (fileHeader == NULL)
    {
        logError("Error: no file header available\n");
        pclose(sourceFD);
        return;
    }
    setNameFileHeader(fileHeader, fileName);

    FILE *star = fopen(tableFile->fileName, "wb");
    if (star == NULL)
    {
        logError("Error: opening tar file '%s'.\n", tableFile->fileName);
        fclose(sourceFD);
        return;
    }

    // Write the file header to the table file
    int tableOffset = getOffsetTableFile(tableFile);
    char buffer[BLOCK_SIZE];

    // moves over the table file to the end of the file headers
    if (fseek(star, tableOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        fclose(star);
        return;
    }

    //
    int previousBlock = -1;
    // get next available block
    int numBlock = getBlockAvailable(tableFile);
    // sets first block
    fileHeader->first = numBlock;
    while (1)
    {
        // reads the block from the source file
        size_t bytesRead = fread(buffer, 1, BLOCK_SIZE, sourceFD);
        if (bytesRead == 0)
        {
            break;
        }

        if (bytesRead == -1)
        {
            logError("Error: reading source file '%s'.\n", fileName);
            fclose(sourceFD);
            break;
        }

        // calculate the offset to write the block in the table file. BLOCK_SIZE + sizeof(int) is the size of a block plus the size of the block number
        // if the block is not consecutive, then move to the next block
        if (previousBlock + 1 != numBlock)
        {

            // !!! be caredul: numBlock can be less than the current block and we need to move back
            int offset = numBlock * (BLOCK_SIZE + sizeof(int));
            if (fseek(star, offset, SEEK_SET) != 0)
            {
                logError("Error: seeking in source file '%s'.\n", fileName);
                fclose(sourceFD);
                break;
            }
        }
        int offset = numBlock * (BLOCK_SIZE + sizeof(int));

        // write the block to the table file .star
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, star);
        //! Tener cuidado de escribir un bloque de tamaño menor al BLOCK_SIZE
        // ! Si el bloque es menor al BLOCK_SIZE, se debe llenar el espacio restante con 0
        if (bytesWritten < bytesRead)
        {
            logError("Error: writing tar file '%s'.\n", tableFile->fileName);
            break;
        }

        // Validar si los bytes leidos son iguales al block size entonces hay que escribir en un nuevo bloque
        // Se necesita el numero de ese siguiente bloque

        // check that bytes read and BLOCK_SIZE are the same
        if (bytesRead < BLOCK_SIZE)
        {
            // write -1 as next block
            int nextBlock = -1;
            fwrite(&nextBlock, sizeof(int), 1, star);
            break;
        }

        previousBlock = numBlock;
        numBlock = getBlockAvailable(tableFile);
        fwrite(&numBlock, sizeof(int), 1, star);
    }

    // TODO : return error code
    tableFile->filesCount++;
    fclose(sourceFD);
    fclose(star);
}

// this retrieves the file header to use, searches it there is one deleted, otherwise creates a new one
FileHeader *getFileHeaderToUse(TableFile *tableFile)
{
    for (int i = 0; i < FILES_NUM; i++)
    {
        if (tableFile->fileHeader[i] == NULL)
        {
            continue;
        }

        if (isFileHeaderAvailable(tableFile->fileHeader[i]))
        {
            return tableFile->fileHeader[i];
        }
    }
    return NULL;
}

int getOffsetTableFile(TableFile *tableFile)
{
    return sizeof(tableFile->fileName) + sizeof(tableFile->blockCount) + sizeof(tableFile->filesCount) + sizeof(FileHeader) * (FILES_NUM + 1);
}

// checks if a block is available in the table file
// first from the free blocks, otherwise creates a new one
// starts from zero
int getBlockAvailable(TableFile *tableFile)
{
    if (tableFile->freeBlocksHeader->first != -1)
    {
        int firstBlock = tableFile->freeBlocksHeader->first;
        // TODO : read in disk the next block
        // open the file
        // seek to the block
        // read the block
        // close the file

        return firstBlock;
    }
    return tableFile->blockCount++;
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

void extractAllFiles(TableFile *tableFile, const char *outputDirectory)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for extractFile\n");
        return;
    }

    FILE *tarFile = fopen(tableFile->fileName, "rb");
    if (tarFile == NULL)
    {
        logError("Error: opening tar file '%s'.\n", tableFile->fileName);
        return;
    }

    char buffer[BLOCK_SIZE];
    int tableOffset = sizeof(FileHeader) * (FILES_NUM + 1);

    for (int i = 0; i < tableFile->filesCount; i++)
    {
        FileHeader *fileHeader = tableFile->fileHeader[i];
        if (fileHeader == NULL || fileHeader->isDeleted)
        {
            continue;
        }

        char outputFilePath[FILE_NAME_SIZE];
        snprintf(outputFilePath, sizeof(outputFilePath), "%s/%s", outputDirectory, fileHeader->name);

        FILE *outputFile = fopen(outputFilePath, "wb");
        if (outputFile == NULL)
        {
            logError("Error: opening output file '%s'.\n", outputFilePath);
            fclose(tarFile);
            return;
        }

        int blockNum = fileHeader->first;
        long tarOffset = tableOffset + (blockNum * BLOCK_SIZE);

        size_t bytesRemaining = fileHeader->size;

        while (bytesRemaining > 0)
        {
            if (fseek(tarFile, tarOffset, SEEK_SET) != 0)
            {
                logError("Error: seeking in tar file '%s'.\n", tableFile->fileName);
                return;
            }

            size_t bytesToRead = (bytesRemaining < BLOCK_SIZE) ? bytesRemaining : BLOCK_SIZE;
            size_t bytesRead = fread(buffer, 1, bytesToRead, tarFile);

            if (bytesRead == 0)
            {
                break;
            }

            if (ferror(tarFile))
            {
                logError("Error: reading tar file '%s'.\n", tableFile->fileName);
                return;
            }

            size_t bytesWritten = fwrite(buffer, 1, bytesRead, outputFile);
            if (bytesWritten < bytesRead)
            {
                logError("Error: writing output file '%s'.\n", outputFilePath);
                break;
            }

            bytesRemaining -= bytesRead;
            tarOffset += BLOCK_SIZE;
        }
        fclose(outputFile);
    }
    fclose(tarFile);
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

    fwrite(&tableFile->fileName, sizeof(tableFile->fileName), 1, file);

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
    TableFile *tableFile = newTableFile(filename);
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        logError("Failed to open file for reading");
        return NULL;
    }

    fread(&tableFile->fileName, sizeof(tableFile->fileName), 1, file);

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

void listFiles(TableFile *tableFile)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for listFiles\n");
        return;
    }

    printf("Files in the table:\n");

    for (int i = 0; i < tableFile->filesCount; i++)
    {
        FileHeader *file = tableFile->fileHeader[i];
        if (file != NULL && file->name != NULL && !file->isDeleted)
        {
            printf("Archivo %d: %s, Tamaño: %u bytes, Primer bloque: %d\n", i + 1, file->name, file->size, file->first);
        }
    }

    if (tableFile->filesCount == 0)
    {
        printf("No hay archivos en la tabla\n");
    }
}