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
void addFile(TableFile *tableFile, char *name, char *tarFileName)
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

    if (name == NULL | tarFileName != NULL)
    {
        logError("Error: File name is null for addFile\n");
        return;
    }
    
    FILE  *tarFileName = fopen(tarFileName, "ab");
    if (tarFileName == NULL)
    {
        logError("Error: opening tar file '%s'.\n", tarFileName);
        fclose(sourceFD);
        return;
    }
    
    // ??? WOULD IT BE CORRECT TO USE A BUFFER OF 256KB?
    int tableOffset = sizeof(FileHeader) * (FILES_NUM + 1);
    char buffer[BLOCK_SIZE];
    File *file = getFileToUse(tableFile);
    setNameFile(file, name);

    if (fseek(tarFileName, tableOffset, SEEK_SET) != 0) { 
            logError("Error: seeking in source file '%s'.\n", name);
            fclose(sourceFD);
            return;
    }

    //Crear función para obtener bloque disponible
    //int numBlock = getBlockAvailable(tarFileName); Debe empezar desde 0
    // Si se crea un bloque, se incrementa el contador de bloques, si no, se usa bloque libre
    while (1)
    {
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

        // TODO: Abrir dos archivos al mismo tiempo, el tar y el archivo actualmente (se pasa por parametro)

        // ! Se debe contemplar que el Numblock obtenido sea menor al bloque en el que estoy posicionado

        //Calcular el offset del bloque
        // int offset = numBlock * (BLOCK_SIZE + sizeof(int));

        // ! Si son bloques consecutivos, no es necesario calcular el offset
        //Se usa offset para el comando fseek sobre el archivo .tar y se va moviendo
        //Escribir el buffer sobre el archivo .tar

        //Validar si los bytes leidos son iguales al block size entonces hay que escribir en un nuevo bloque
        //Se necesita el numero de ese siguiente bloque

        //! Tener cuidado de escribir un bloque de tamaño menor al BLOCK_SIZE
        // ! Si el bloque es menor al BLOCK_SIZE, se debe llenar el espacio restante con 0
        
        //Llamar a la función getBlockAvailable para obtener el siguiente bloque disponible y escribir en el Tar
        // numBlock = getBlockAvailable(tarFileName);
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

void listFiles(TableFile *tableFile) {
    if (tableFile == NULL) {
        logError("Error: TableFile is null for listFiles\n");
        return;
    }

    printf("Files in the table:\n");

    for (int i = 0; i < tableFile->filesCount; i++) {
        FileHeader *file = tableFile->fileHeader[i];
        if (file != NULL && file->name != NULL && !file->isDeleted) {
            printf("Archivo %d: %s, Tamaño: %zu bytes, Primer bloque: %d\n", i + 1, file->name, file->size, file->first);
        }
    }

    if (tableFile->filesCount == 0) {
        printf("No hay archivos en la tabla\n");
    }
}