#include <fcntl.h>
#include <string.h>
#include <unistd.h> // For ftruncate

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

    logInfo("Opening table file '%s' with mode '%s'.\n", tableFile->fileName, mode);
    tableFile->file = fopen(tableFile->fileName, mode);
    if (tableFile->file == NULL)
    {
        logError("Error: opening table file '%s' with mode '%s'.\n", tableFile->fileName, mode);
        return;
    }
}

void closeTableFile(TableFile *tableFile)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for closeTableFile\n");
        return;
    }

    if (tableFile->file == NULL)
    {
        logError("Error: File is null for closeTableFile\n");
        return;
    }

    fclose(tableFile->file);
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

    // remove the path from the file name
    const char *lastSlash = strrchr(fileName, '/');
    const char *fileNameWithoutPath;
    if (lastSlash != NULL)
    {
        fileNameWithoutPath = lastSlash + 1;
    }
    else
    {
        fileNameWithoutPath = fileName;
    }

    FileHeader *exists = getFileHeader(tableFile, fileNameWithoutPath);
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

    setNameFileHeader(fileHeader, fileNameWithoutPath);

    // Write the file header to the table file
    int tableOffset = getOffsetTableFile();

    // moves over the table file to the end of the file headers
    if (fseek(tableFile->file, tableOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        return;
    }

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
        fileBlock->size = bytesRead;

        if (bytesRead == -1)
        {
            logError("Error: reading source file '%s'.\n", fileName);
            fclose(sourceFD);
            break;
        }

        // always move from the beginning of the file
        int offset = getOffsetTableFile() + currentBlock * (getOffsetFileBlock());
        if (fseek(tableFile->file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", fileName);
            fclose(sourceFD);
            return;
        }

        fileHeader->size += bytesRead;
        fileHeader->last = currentBlock;
        fileHeader->totalBlocks++;

        // check that bytes read and BLOCK_SIZE are the same,
        // if bytes read is less than BLOCK_SIZE, then it is the last block
        if (bytesRead < BLOCK_SIZE)
        {
            fileBlock->next = -1;
            serializeFileBlock(fileBlock, tableFile->file);
            break;
        }

        currentBlock = getBlockAvailable(tableFile);
        fileBlock->next = currentBlock;
        // write the block to the table file .star
        //! Tener cuidado de escribir un bloque de tamaño menor al BLOCK_SIZE
        // ! Si el bloque es menor al BLOCK_SIZE, se debe llenar el espacio restante con 0
        serializeFileBlock(fileBlock, tableFile->file);
    }

    logInfo("File '%s' added to table file '%s'.\n", fileName, tableFile->fileName);

    // we need to update the star with the information of the file header
    // move to the beginning of the file. SEEK_SET
    // move to the position of the file header. fileHeaderOffset
    int fileHeaderOffset = sizeof(tableFile->fileName) + (fileHeader->index * sizeof(FileHeader));
    if (fseek(tableFile->file, fileHeaderOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        return;
    }

    // write the file header
    serializeFileHeader(fileHeader, tableFile->file);

    // TODO : return error code
    tableFile->filesCount++;
    // update filesCount in the table file
    int filesCountOffset = sizeof(tableFile->fileName) + ((FILES_NUM + 1) * sizeof(FileHeader));
    if (fseek(tableFile->file, filesCountOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        return;
    }

    fwrite(&tableFile->filesCount, sizeof(int), 1, tableFile->file);
    // update blockCount in the table file
    fwrite(&tableFile->blockCount, sizeof(int), 1, tableFile->file);

    fclose(sourceFD);
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

int getOffsetTableFile()
{
    return sizeof(char[FILE_NAME_SIZE]) + /*blockCount*/ sizeof(int) + /*filesCount*/ sizeof(int) + sizeof(FileHeader) * (FILES_NUM + 1);
}

// checks if a block is available in the table file
// first from the free blocks, otherwise takes the counter of blockCount
// starts from zero
// TODO : use -1 to validate that the block is not available
int getBlockAvailable(TableFile *tableFile)
{
    if (tableFile->freeBlocksHeader->first != -1)
    {
        int firstBlock = tableFile->freeBlocksHeader->first;
        // open the file
        FILE *file = fopen(tableFile->fileName, "rb");
        if (file == NULL)
        {
            logError("Error: opening table file '%s' with mode '%s'.\n", tableFile->fileName, "rb");
            return -1;
        }
        // seek to the block
        int offset = getOffsetTableFile() + firstBlock * (getOffsetFileBlock());
        if (fseek(file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", tableFile->fileName);
            fclose(file);
            return -1;
        }
        // read the block
        FileBlock *fileBlock = deserializeFileBlock(file);
        if (fileBlock == NULL)
        {
            fclose(file);
            return -1;
        }
        // update the free blocks header
        tableFile->freeBlocksHeader->first = fileBlock->next;
        tableFile->freeBlocksHeader->totalBlocks -= 1;
        tableFile->freeBlocksHeader->size -= fileBlock->size;
        fclose(file);

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

    int tableOffset = getOffsetTableFile();

    int filesToExtract = tableFile->filesCount;

    for (int i = 0; i < FILES_NUM; i++)
    {
        if (filesToExtract == 0)
        {
            break;
        }

        // moves over the table file to the end of the file headers
        if (fseek(tableFile->file, tableOffset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", tableFile->fileName);
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
            return;
        }

        size_t bytesRemaining = fileHeader->size;

        FileBlock *fileBlock = newFileBlock();
        setNextFileBlock(fileBlock, fileHeader->first);
        while (1)
        {

            // always move from the beginning of the file
            int offset = getOffsetTableFile() + fileBlock->next * (getOffsetFileBlock());
            if (fseek(tableFile->file, offset, SEEK_SET) != 0)
            {
                logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                fclose(outputFile);
                return;
            }

            size_t bytesToWrite = (bytesRemaining < BLOCK_SIZE) ? bytesRemaining : BLOCK_SIZE;
            // size_t bytesRead = fread(buffer, 1, bytesToRead, tarFile);
            // add bytesRead into deserializeFileBlock
            //
            free(fileBlock);
            fileBlock = deserializeFileBlock(tableFile->file);
            if (fileBlock == NULL)
            {
                break;
            }

            if (ferror(tableFile->file))
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
        logInfo("File '%s' extracted to '%s'.\n", fileHeader->name, outputFilePath);
        filesToExtract--;
    }
}

// Function to serialize the TableFile structure
void serializeTableFile(TableFile *tableFile, const char *outputFile)
{
    if (tableFile->file == NULL)
    {
        logError("Failed to open file for writing");
        return;
    }

    logInfo("Serializing table file '%s'.\n", toStringTableFile(tableFile));
    fwrite(&tableFile->fileName, sizeof(tableFile->fileName), 1, tableFile->file);

    // serialize file headers
    for (int i = 0; i < FILES_NUM; i++)
    {
        serializeFileHeader(tableFile->fileHeader[i], tableFile->file);
    }

    // serialize free blocks header
    serializeFileHeader(tableFile->freeBlocksHeader, tableFile->file);

    fwrite(&tableFile->filesCount, sizeof(tableFile->filesCount), 1, tableFile->file);
    fwrite(&tableFile->blockCount, sizeof(tableFile->blockCount), 1, tableFile->file);
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

    logInfo("Archivos en la tabla:\n");

    int archivosListados = 0;

    for (int i = 0; i < FILES_NUM; i++)
    {
        FileHeader *fileHeader = tableFile->fileHeader[i];

        if (fileHeader != NULL && !isFileHeaderAvailable(fileHeader))
        {
            int *blockList = getBlockList(fileHeader, tableFile->file);

            printFileHeader(fileHeader, blockList);
            archivosListados++;
        }
        // TODO : would be nice to list all blocks of the file
    }

    int *blockList = getBlockList(tableFile->freeBlocksHeader, tableFile->file);

    printFileHeader(tableFile->freeBlocksHeader, blockList);

    if (archivosListados == 0)
    {
        logInfo("No hay archivos en la tabla\n");
    }
}

void delete(TableFile *tableFile, const char *fileName)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for delete\n");
        return;
    }

    if (tableFile->file == NULL)
    {
        logError("Error: File is null for delete\n");
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
    tableFile->blockCount -= fileHeader->totalBlocks;
    // add into the free blocks
    // tableFile->freeBlocksHeader->last = fileHeader->last;
    tableFile->freeBlocksHeader->size += fileHeader->size;
    tableFile->freeBlocksHeader->totalBlocks += fileHeader->totalBlocks;

    if (tableFile->freeBlocksHeader->first == -1)
    {
        tableFile->freeBlocksHeader->first = fileHeader->first;
    }
    else // when there are already free blocks
    {
        // seek the last block in free blocks
        int offset = getOffsetTableFile() + tableFile->freeBlocksHeader->last * (getOffsetFileBlock());

        if (fseek(tableFile->file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", tableFile->fileName);
            return;
        }

        // read the last block
        FileBlock *lastFreeBlock = deserializeFileBlock(tableFile->file);

        // go at the beginning of the current read block
        if (fseek(tableFile->file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", tableFile->fileName);
            return;
        }
        lastFreeBlock->next = fileHeader->first;

        // write the last block
        serializeFileBlock(lastFreeBlock, tableFile->file);

        // close the file
    }

    logInfo("File '%s' deleted\n", fileName);

    tableFile->freeBlocksHeader->last = fileHeader->last;

    // write the file header deleted
    int offsetFileHeader = sizeof(tableFile->fileName) + (fileHeader->index * sizeof(FileHeader));
    if (fseek(tableFile->file, offsetFileHeader, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        return;
    }
    serializeFileHeader(fileHeader, tableFile->file);

    writeOrderedBlockList(tableFile->freeBlocksHeader, tableFile->file);

    // write file header for the free blocks
    int offsetFreeBlocksHeader = sizeof(tableFile->fileName) + (FILES_NUM * sizeof(FileHeader));
    if (fseek(tableFile->file, offsetFreeBlocksHeader, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        return;
    }
    serializeFileHeader(tableFile->freeBlocksHeader, tableFile->file);

    fwrite(&tableFile->filesCount, sizeof(int), 1, tableFile->file);
    fwrite(&tableFile->blockCount, sizeof(int), 1, tableFile->file);
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

// this is a copy of addFile, but it will update the file
// TODO : if update is smaller than the original file, we need to add the blocks to the free blocks
void update(TableFile *tableFile, const char *fileName)
{
    if (tableFile == NULL)
    {
        logError("Error: TableFile is null for addFile\n");
        return;
    }

    if (fileName == NULL)
    {
        logError("Error: File name is null for addFile\n");
        return;
    }

    // remove the path from the file name
    const char *lastSlash = strrchr(fileName, '/');
    const char *fileNameWithoutPath;
    if (lastSlash != NULL)
    {
        fileNameWithoutPath = lastSlash + 1;
    }
    else
    {
        fileNameWithoutPath = fileName;
    }

    FileHeader *fileHeader = getFileHeader(tableFile, fileNameWithoutPath);
    if (fileHeader == NULL)
    {
        logError("Error: File '%s' already exists\n", fileNameWithoutPath);
        return;
    }

    FILE *sourceFD = fopen(fileName, "rb");
    if (!sourceFD)
    {
        logError("Error: opening source file '%s'.\n", fileName);
        return;
    }

    // Write the file header to the table file
    int tableOffset = getOffsetTableFile();

    // moves over the table file to the end of the file headers
    if (fseek(tableFile->file, tableOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        return;
    }

    // get next available block
    int currentBlock = fileHeader->first;
    int oldTotalBlocks = fileHeader->totalBlocks;

    tableFile->blockCount -= fileHeader->totalBlocks;
    fileHeader->size = 0;
    fileHeader->totalBlocks = 0;
    int last = fileHeader->last;
    while (1)
    {

        FileBlock *fileBlock = newFileBlock();
        // reads the block from the source file
        size_t bytesRead = fread(fileBlock->data, 1, BLOCK_SIZE, sourceFD);
        if (bytesRead == 0)
        {
            break;
        }
        fileBlock->size = bytesRead;

        if (bytesRead == -1)
        {
            logError("Error: reading source file '%s'.\n", fileName);
            fclose(sourceFD);
            break;
        }
        // always move from the beginning of the file
        int offset = getOffsetTableFile() + currentBlock * (getOffsetFileBlock());
        if (fseek(tableFile->file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", fileName);
            fclose(sourceFD);
            return;
        }

        // this will be valid while the file has enough blocks
        int nextUpdateBlock;
        size_t bytesReadNext = fread(&nextUpdateBlock, sizeof(nextUpdateBlock), 1, tableFile->file);
        if (bytesReadNext < 1)
        {
            logError("Error: reading next block\n");
            return;
        }
        // go back the read block
        if (fseek(tableFile->file, -sizeof(nextUpdateBlock), SEEK_CUR) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", fileName);
            fclose(sourceFD);
            return;
        }
        // this is to get a new block if the update is greater than the original file
        // sets nextUpdateBlock as the current block as long it is not -1
        nextUpdateBlock = (nextUpdateBlock == -1) ? getBlockAvailable(tableFile) : nextUpdateBlock;

        fileHeader->size += bytesRead;
        fileHeader->last = currentBlock;
        fileHeader->totalBlocks++;

        // check that bytes read and BLOCK_SIZE are the same,
        // if bytes read is less than BLOCK_SIZE, then it is the last block
        if (bytesRead < BLOCK_SIZE)
        {
            if (oldTotalBlocks > fileHeader->totalBlocks)
            {
                logDebug("File '%s' updated has less block.\n", fileName);
                if (tableFile->freeBlocksHeader->first == -1)
                {
                    tableFile->freeBlocksHeader->first = nextUpdateBlock;
                }
                else
                {
                    // move to the last block
                    int offsetLastBlock = getOffsetTableFile() + tableFile->freeBlocksHeader->last * getOffsetFileBlock();
                    if (fseek(tableFile->file, offsetLastBlock, SEEK_SET) != 0)
                    {
                        logError("Error: seeking in source file '%s'.\n", fileName);
                        fclose(sourceFD);
                        return;
                    }

                    // write the next block
                    size_t bytesWritten = fwrite(&nextUpdateBlock, sizeof(nextUpdateBlock), 1, tableFile->file);
                    if (bytesWritten < 1)
                    {
                        logError("Error: writing next block\n");
                        fclose(sourceFD);
                        return;
                    }
                }

                // add remaining blocks to the free blocks
                tableFile->freeBlocksHeader->totalBlocks += oldTotalBlocks - fileHeader->totalBlocks;
                tableFile->freeBlocksHeader->last = last;
                int offsetFreeBlocksHeader = sizeof(tableFile->fileName) + (FILES_NUM * sizeof(FileHeader));
                if (fseek(tableFile->file, offsetFreeBlocksHeader, SEEK_SET) != 0)
                {
                    logError("Error: seeking in source file '%s'.\n", fileName);
                    fclose(sourceFD);
                    return;
                }

                serializeFileHeader(tableFile->freeBlocksHeader, tableFile->file);
            }
            else
            {

                logDebug("File '%s' updated has more block.\n", fileName);
                fileBlock->next = -1;
                serializeFileBlock(fileBlock, tableFile->file);
            }
            break;
        }

        currentBlock = nextUpdateBlock;
        fileBlock->next = currentBlock;
        // write the block to the table file .star
        serializeFileBlock(fileBlock, tableFile->file);
    }

    logInfo("File '%s' updated in table file '%s'.\n", fileName, tableFile->fileName);

    // we need to update the star with the information of the file header
    // move to the beginning of the file. SEEK_SET
    // move to the position of the file header. fileHeaderOffset
    int fileHeaderOffset = sizeof(tableFile->fileName) + (fileHeader->index * sizeof(FileHeader));
    if (fseek(tableFile->file, fileHeaderOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        return;
    }

    // write the file header
    serializeFileHeader(fileHeader, tableFile->file);

    // update filesCount in the table file
    int filesCountOffset = sizeof(tableFile->fileName) + ((FILES_NUM + 1) * sizeof(FileHeader)) + sizeof(int);
    if (fseek(tableFile->file, filesCountOffset, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", fileName);
        fclose(sourceFD);
        return;
    }

    tableFile->blockCount += fileHeader->totalBlocks;

    // update blockCount in the table file
    fwrite(&tableFile->blockCount, sizeof(int), 1, tableFile->file);

    fclose(sourceFD);
}

void pack(TableFile *tableFile) // also removes the deleted files
{
    // get list of freeBlocks, this list will be ordered from smaller to bigger
    int *freeBlocks = getBlockList(tableFile->freeBlocksHeader, tableFile->file);
    logInfo("Packing table file '%s'.\n", tableFile->fileName);

    // iterate over all file headers
    for (int i = 0; i < FILES_NUM; i++)
    {
        FileHeader *fileHeader = tableFile->fileHeader[i];
        if (fileHeader == NULL)
        {
            continue;
        }

        if (isFileHeaderAvailable(fileHeader))
        {
            continue;
        }

        if (tableFile->freeBlocksHeader->totalBlocks == 0)
        {
            break;
        }

        // get the ordered block list
        int *fileHeaderBlocks = getBlockList(fileHeader, tableFile->file);

        int previousBlock = -1;
        int currentBlock = -1;

        int movedBlocks = 0;
        // iterate over all blocks of the file header
        // this iteration will remove and add a block from the freeBlocks
        for (int j = 0; j < fileHeader->totalBlocks; j++)
        {
            currentBlock = fileHeaderBlocks[j];
            int newPositionBlock = freeBlocks[0];
            if (newPositionBlock > currentBlock) // will never be equal because blocks has a unique number
            {
                continue;
            }

            if (j == 0)
            {
                fileHeader->first = newPositionBlock;
            }

            fileHeader->last = freeBlocks[0];
            movedBlocks++;

            // when previous is valid we need to go back and update its next block
            if (previousBlock != -1)
            {
                // move to the previous block
                int offset = getOffsetTableFile() + previousBlock * getOffsetFileBlock();
                if (fseek(tableFile->file, offset, SEEK_SET) != 0)
                {
                    logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                    return;
                }

                // write the next block
                // the first int is the next block
                size_t bytesWritten = fwrite(&newPositionBlock, sizeof(newPositionBlock), 1, tableFile->file);
                if (bytesWritten < 1)
                {
                    logError("Error: writing next block\n");
                    return;
                }
            }

            // moves to the current block
            int offset = getOffsetTableFile() + currentBlock * (getOffsetFileBlock());
            if (fseek(tableFile->file, offset, SEEK_SET) != 0)
            {
                logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                return;
            }

            // read the block
            FileBlock *currentFileBlock = deserializeFileBlock(tableFile->file);

            // move to new position block given by newPositionBlock
            int newOffset = getOffsetTableFile() + newPositionBlock * (getOffsetFileBlock());
            if (fseek(tableFile->file, newOffset, SEEK_SET) != 0)
            {
                logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                return;
            }

            // write the block
            serializeFileBlock(currentFileBlock, tableFile->file);

            // as first block was used now i need to move the freeBlocks[i+1] into freeBlocks[i]
            // and once current block has a space in freeBlocks, then i need to add and stop

            // move k +1 to k
            // insert currentBlock when currentBlock < freeBlocks[k+1]
            // sets currentBlock as the last block if it is bigger than the last block
            for (int k = 0; k < tableFile->freeBlocksHeader->totalBlocks; k++)
            {
                if (currentBlock < freeBlocks[k])
                {
                    freeBlocks[k] = currentBlock;
                    break;
                }
                freeBlocks[k] = freeBlocks[k + 1];
                if (k == tableFile->freeBlocksHeader->totalBlocks - 1)
                {
                    freeBlocks[k] = currentBlock;
                }
            }
            previousBlock = newPositionBlock;
        }

        if (movedBlocks > 0)
        {
            // write the file header
            int offsetFileHeader = sizeof(tableFile->fileName) + (fileHeader->index * sizeof(FileHeader));
            if (fseek(tableFile->file, offsetFileHeader, SEEK_SET) != 0)
            {
                logError("Error: seeking in source file '%s'.\n", fileHeader->name);
                return;
            }
            serializeFileHeader(fileHeader, tableFile->file);
        }
    }

    // at this moment all free blocks are at the end of the file
    // so we need to update the freeBlocksHeader
    logInfo("Free blocks: %d\n", tableFile->freeBlocksHeader->totalBlocks);
    resetFileHeader(tableFile->freeBlocksHeader);

    // write the freeBlocksHeader
    int offsetFreeBlocksHeader = sizeof(tableFile->fileName) + (FILES_NUM * sizeof(FileHeader));
    if (fseek(tableFile->file, offsetFreeBlocksHeader, SEEK_SET) != 0)
    {
        logError("Error: seeking in source file '%s'.\n", tableFile->fileName);
        return;
    }
    serializeFileHeader(tableFile->freeBlocksHeader, tableFile->file);

    // Offset from which you want to keep the file's content
    int finalOffset = getOffsetTableFile() + tableFile->blockCount * getOffsetFileBlock();

    // Get the file descriptor from FILE *
    int fd = fileno(tableFile->file);
    if (fd == -1)
    {
        logError("Error getting file descriptor");
        return;
    }

    // Truncate the file using ftruncate
    if (ftruncate(fd, finalOffset) == -1)
    {
        logError("Error truncating file");
        return;
    }
}

int *getBlockList(FileHeader *fileHeader, FILE *file)
{
    if (file == NULL)
    {
        logError("Error: File is null for getBlockList\n");
        return NULL;
    }

    if (fileHeader == NULL)
    {
        logError("Error: FileHeader is null for getBlockList\n");
        return NULL;
    }

    if (fileHeader->totalBlocks == 0)
    {
        return NULL;
    }

    // moves to the first block in the fileHeader->first
    int currentBlock = fileHeader->first;
    int *blockList = (int *)malloc(sizeof(int) * fileHeader->totalBlocks);

    for (int i = 0; i < fileHeader->totalBlocks; i++)
    {
        // moves to the block
        int offset = getOffsetTableFile() + currentBlock * (getOffsetFileBlock());
        if (fseek(file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", fileHeader->name);
            return NULL;
        }

        // reads the block
        FileBlock *fileBlock = deserializeFileBlock(file);
        if (fileBlock == NULL)
        {
            // fclose(star);
            return NULL;
        }

        blockList[i] = currentBlock;
        currentBlock = fileBlock->next;
    }

    return blockList;
}

int *orderedBlockList(FileHeader *FileHeader, FILE *file)
{
    int *blockList = getBlockList(FileHeader, file);
    // order the list of blocks from smaller to bigger

    for (int i = 0; i < FileHeader->totalBlocks; i++)
    {
        for (int j = i + 1; j < FileHeader->totalBlocks; j++)
        {
            if (blockList[i] > blockList[j])
            {
                int temp = blockList[i];
                blockList[i] = blockList[j];
                blockList[j] = temp;
            }
        }
    }

    return blockList;
}

// this will write the ordered block list
// and will udpate the first and last block in the file header
void writeOrderedBlockList(FileHeader *fileHeader, FILE *file)
{

    if (fileHeader == NULL)
    {
        logError("Error: blockList is null for writeBlockList\n");
        return;
    }

    if (file == NULL)
    {
        logError("Error: File is null for writeBlockList\n");
        return;
    }

    if (file == NULL)
    {
        logError("Error: File is null for writeBlockList\n");
        return;
    }

    int *blockList = orderedBlockList(fileHeader, file);

    int currentBlock = -1;
    int nextBlock = -1;

    fileHeader->first = blockList[0];
    // iterate over the blockList
    for (int i = 0; i < fileHeader->totalBlocks; i++)
    {
        currentBlock = blockList[i];
        fileHeader->last = currentBlock;
        // there are another block
        if (i < fileHeader->totalBlocks - 1)
        {
            nextBlock = blockList[i + 1];
        }
        else
        {
            nextBlock = -1;
        }

        // move to the block
        int offset = getOffsetTableFile() + currentBlock * (getOffsetFileBlock());
        if (fseek(file, offset, SEEK_SET) != 0)
        {
            logError("Error: seeking in source file '%s'.\n", fileHeader->name);
            return;
        }

        // write the block
        size_t bytesWritten = fwrite(&nextBlock, sizeof(nextBlock), 1, file);
    }
}

char *toStringTableFile(TableFile *tableFile)
{
    char *result = (char *)malloc(1000);
    snprintf(result, 1000, "TableFile: %s\n", tableFile->fileName);
    // add number of blocks and files
    snprintf(result, 1000, "%sBlocks: %d\n", result, tableFile->blockCount);
    snprintf(result, 1000, "%sFiles: %d\n", result, tableFile->filesCount);
    return result;
}