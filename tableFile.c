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
        tableFile->fileHeader[i]->index = i;
    }

    tableFile->freeBlocksHeader = newFileHeader();
    setNameFileHeader(tableFile->freeBlocksHeader, "freeBlocksHeader");

    tableFile->filesCount = 0;
    tableFile->blockCount = 0;

    return tableFile;
}

void openTableFile(TableFile *tableFile, const char *mode)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for openTableFile\n");
        return;
    }

    if (mode == NULL)
    {
        logError("Error: Mode is null for openTableFile\n");
        return;
    }

    tableFile->file = fopen(tableFile->fileName, mode);
    if (tableFile->file == NULL)
    {
        logError("Error: opening table file '%s' with mode '%s'.\n", tableFile->fileName, mode);
        return;
    }
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

    FileHeader *exists = getFileHeader(tableFile, fileName);
    if (exists != NULL)
    {
        logError("Error: File '%s' already exists\n", fileName);
        return;
    }

    FILE *sourceFD = fopen(fileName, "rb");
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
    // remove the path from the file name
    const char *lastSlash = strrchr(fileName, '/');
    if (lastSlash != NULL)
    {
        fileName = lastSlash + 1;
    }

    setNameFileHeader(fileHeader, fileName);

    FILE *star = fopen(tableFile->fileName, "rb+");
    if (star == NULL)
    {
        logError("Error: opening tar file '%s'.\n", tableFile->fileName);
        fclose(sourceFD);
        return;
    }

    // Write the file header to the table file
    int tableOffset = getOffsetTableFile(tableFile);

    // moves over the table file to the end of the file headers
    if (fseek(star, tableOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        fclose(star);
        return;
    }

    // previous and current block to write the block number
    int previousBlock = -1;
    // get next available block
    int currentBlock = getBlockAvailable(tableFile);
    // sets first block
    fileHeader->first = currentBlock;
    while (1)
    {
        FileBlock *fileBlock = newFileBlock();
        // reads the block from the source file
        size_t bytesRead = fread(fileBlock->data, 1, BLOCK_SIZE, sourceFD);
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
        // TODO : this section convert to a function, and use it in extractAllFiles too
        if (previousBlock + 1 != currentBlock)
        {

            // !!! be careful: currentBlock can be less than the current block and we need to move backwards
            // move forward to the next block
            if (currentBlock > previousBlock)
            {
                // TODO : pending to test when delete is implemented
                // !!! and offset move is not always from the  end of the table file
                // 3 -> 7  it will be at the beginning of the fourth block
                // so we need to move forward to seventh block
                // from 4 to 5, from 5 to 6, from 6 to 7
                // so it moves 3 blocks
                // 7 - 3 - 1 = 4
                // 7 : currentBlock
                // 3 : previousBlock
                // 1 : moved by the write of the previous block
                int blocksToMove = currentBlock - previousBlock - 1;
                logDebug("moving forward previousBlock: %d, currentBlock: %d, blocksToMove: %d\n", previousBlock, currentBlock, blocksToMove);
                int offset = blocksToMove * (BLOCK_SIZE + sizeof(int));
                if (fseek(star, offset, SEEK_CUR) != 0)
                {
                    logError("Error: seeking in source file '%s'.\n", fileName);
                    fclose(sourceFD);
                    break;
                }
            }
            else
            {
                // TODO : pending to test when delete is implemented
                // move backward to the next block
                // from 7 to 3
                // it will be at the beginning of the eighth block
                // so we need to move back to the third block
                // from 7 to 6, from 6 to 5, from 5 to 4, from 4 to 3
                // so it moves 4 blocks
                // 3 - 1 - 7 = -3
                int blocksToMove = previousBlock - 1 - currentBlock;
                logDebug("moving backwards previousBlock: %d, currentBlock: %d, blocksToMove: %d\n", previousBlock, currentBlock, blocksToMove);
                int offset = blocksToMove * (BLOCK_SIZE + sizeof(int));
                if (fseek(star, offset, SEEK_CUR) != 0)
                {
                    logError("Error: seeking in source file '%s'.\n", fileName);
                    fclose(sourceFD);
                    break;
                }
            }
        }

        fileHeader->size += bytesRead;

        // check that bytes read and BLOCK_SIZE are the same,
        // if bytes read is less than BLOCK_SIZE, then it is the last block
        if (bytesRead < BLOCK_SIZE)
        {
            fileBlock->next = -1;
            serializeFileBlock(fileBlock, star);
            break;
        }

        fileHeader->last = currentBlock;
        fileHeader->totalBlocks++;
        previousBlock = currentBlock;
        currentBlock = getBlockAvailable(tableFile);
        fileBlock->next = currentBlock;
        // write the block to the table file .star
        //! Tener cuidado de escribir un bloque de tamaño menor al BLOCK_SIZE
        // ! Si el bloque es menor al BLOCK_SIZE, se debe llenar el espacio restante con 0
        serializeFileBlock(fileBlock, star);
    }

    // we need to update the star with the information of the file header
    // move to the beginning of the file. SEEK_SET
    // move to the position of the file header. fileHeaderOffset
    int fileHeaderOffset = sizeof(tableFile->fileName) + (fileHeader->index * sizeof(FileHeader));
    if (fseek(star, fileHeaderOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        fclose(star);
        return;
    }

    // write the file header
    serializeFileHeader(fileHeader, star);

    // TODO : return error code
    tableFile->filesCount++;
    // update filesCount in the table file
    int filesCountOffset = sizeof(tableFile->fileName) + ((FILES_NUM + 1) * sizeof(FileHeader));
    if (fseek(star, filesCountOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        fclose(star);
        return;
    }

    fwrite(&tableFile->filesCount, sizeof(int), 1, star);

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
        // TODO : implement once the commant DELETE is implemented
        // open the file
        // seek to the block
        // read the block
        // close the file

        return firstBlock;
    }
    return tableFile->blockCount++;
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

    // TODO :  open it once and close it once
    FILE *tarFile = fopen(tableFile->fileName, "rb");
    if (tarFile == NULL)
    {
        logError("Error: opening tar file '%s'.\n", tableFile->fileName);
        return;
    }

    int tableOffset = getOffsetTableFile(tableFile);

    int filesToExtract = tableFile->filesCount;

    for (int i = 0; i < FILES_NUM; i++)
    {
        if (filesToExtract == 0)
        {
            break;
        }

        // moves over the table file to the end of the file headers
        if (fseek(tarFile, tableOffset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", tableFile->fileName);
            fclose(tarFile);
            return;
        }

        FileHeader *fileHeader = tableFile->fileHeader[i];
        if (fileHeader == NULL || isFileHeaderAvailable(fileHeader))
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

        int previousBlock = -1;

        size_t bytesRemaining = fileHeader->size;

        FileBlock *fileBlock = newFileBlock();
        setNextFileBlock(fileBlock, fileHeader->first);
        while (1)
        {

            // it can move forward and backars
            if (previousBlock + 1 != fileBlock->next)
            {
                // !!! be careful: currentBlock can be less than the current block and we need to move backwards
                // move forward to the next block
                if (fileBlock->next > previousBlock)
                {
                    // TODO : pending to test when delete is implemented
                    // !!! and offset move is not always from the  end of the table file
                    int blocksToMove = fileBlock->next - previousBlock - 1;
                    logDebug("moving forward previousBlock: %d, currentBlock: %d, blocksToMove: %d\n", previousBlock, fileBlock->next, blocksToMove);
                    int offset = blocksToMove * (BLOCK_SIZE + sizeof(int));
                    if (fseek(tarFile, offset, SEEK_CUR) != 0)
                    {
                        logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                        fclose(outputFile);
                        break;
                    }
                }
                else
                {
                    // TODO : pending to test when delete is implemented
                    int blocksToMove = previousBlock - 1 - fileBlock->next;
                    logDebug("moving backwards previousBlock: %d, currentBlock: %d, blocksToMove: %d\n", previousBlock, fileBlock->next, blocksToMove);
                    int offset = blocksToMove * (BLOCK_SIZE + sizeof(int));
                    if (fseek(tarFile, offset, SEEK_CUR) != 0)
                    {
                        logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                        fclose(outputFile);
                        break;
                    }
                }
            }

            previousBlock = fileBlock->next;

            size_t bytesToWrite = (bytesRemaining < BLOCK_SIZE) ? bytesRemaining : BLOCK_SIZE;
            // size_t bytesRead = fread(buffer, 1, bytesToRead, tarFile);
            // add bytesRead into deserializeFileBlock
            //
            free(fileBlock);
            fileBlock = deserializeFileBlock(tarFile);
            if (fileBlock == NULL)
            {
                break;
            }

            if (ferror(tarFile))
            {
                logError("Error: reading tar file '%s'.\n", tableFile->fileName);
                return;
            }

            // just writes the data, no need for the next block
            size_t bytesWritten = fwrite(fileBlock->data, 1, bytesToWrite, outputFile);
            if (bytesWritten < bytesToWrite)
            {
                logError("Error: writing output file '%s'.\n", outputFilePath);
                break;
            }
            // i think it is not necessary
            bytesRemaining -= bytesWritten;

            // it is the last block
            // so move the offset read to the next one
            if (fileBlock->next == -1)
            {
                break;
            }
        }
        fclose(outputFile);
        filesToExtract--;
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

void delete(TableFile *tableFile, const char *fileName)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for delete\n");
        return;
    }

    if (fileName == NULL)
    {
        logError("Error: File name is null for delete\n");
        return;
    }

    FileHeader *fileHeader = getFileHeader(tableFile, fileName);
    if (fileHeader == NULL)
    {
        logError("Error: File '%s' not found\n", fileName);
        return;
    }

    // sets the file as deleted
    fileHeader->isDeleted = 1;
    // decrease the files count
    tableFile->filesCount--;
    // add into the free blocks
    if (tableFile->freeBlocksHeader->first == -1)
    {
        tableFile->freeBlocksHeader->first = fileHeader->first;
    }
    else
    {
        // TODO : pending to implement
        // open the file
        // seek to the last block
        // lastBlock->next = fileHeader->first
        // write the next block
        // close the file
    }
    // write the freeBlocksHeader updated
}

FileHeader *getFileHeader(TableFile *tableFile, const char *fileName)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for fileExists\n");
        return NULL;
    }

    if (fileName == NULL)
    {
        logError("Error: File name is null for fileExists\n");
        return NULL;
    }

    int filesToSearch = tableFile->filesCount;

    if (filesToSearch == 0)
    {
        return NULL;
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
            return tableFile->fileHeader[i];
        }

        filesToSearch--;
    }
    return NULL;
}