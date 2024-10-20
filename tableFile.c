#include "tableFile.h"

TableFile *newTableFile()
{
    TableFile *tableFile = (TableFile *)malloc(sizeof(TableFile));
    if (tableFile == NULL)
    {
        return NULL;
    }

    tableFile->FreeBlocks = NULL;

    return tableFile;
}

void writeTableFile(TableFile *tableFile, char *outputFile)
{
    // no idea was b for
    // FILE *file = fopen(outputFile, "wb");
    FILE *file = fopen(outputFile, "w");
    if (file == NULL)
    {
        printf("Error: No se pudo abrir el archivo %s\n", outputFile);
        return;
    }

    int result = fwrite(tableFile, sizeof(TableFile), 1, file);
    if (result == 0)
    {
        printf("Error writing to the given file\n");
        exit(1);
    }
    else
    {
        printf("TableFile is written to the file successfully!\n");
    }

    fclose(file);
}

TableFile *loadTableFile(char *inputFile)
{
    FILE *file = fopen(inputFile, "r");
    if (file == NULL) {
        printf("Error: No se pudo abrir el archivo %s\n", inputFile);
        return NULL;
    }

    TableFile *tableFile = (TableFile *)malloc(sizeof(TableFile));
    if(fread(tableFile, sizeof(TableFile), 1, file) == 0) {
        printf("Error reading from the given file\n", inputFile);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return tableFile;
}
