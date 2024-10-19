#include "tableFile.h"

TableFile *newTableFile()
{
    TableFile *tableFile = (TableFile *)malloc(sizeof(TableFile));
    if (tableFile == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < FILES_NUM; i++)
    {
        tableFile->Files[i] = newFile(NULL);
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
