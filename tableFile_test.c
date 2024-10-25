#include <assert.h>
#include <string.h>

#include "tableFile.h"
#include "verbose.h"

void test_newTableFile()
{
    TableFile *tableFile = newTableFile();
    assert(tableFile != NULL);
    assert(tableFile->filesCount == 0);
    for (int i = 0; i < FILES_NUM; i++)
    {
        assert(tableFile->fileHeader[i] != NULL);
        assert(tableFile->fileHeader[i]->first == -1);
        assert(tableFile->fileHeader[i]->isDeleted == 0);
        for (int j = 0; j < FILE_NAME_SIZE; j++)
        {
            assert(tableFile->fileHeader[i]->name[j] == '\0');
        }
    }
    assert(tableFile->freeBlocksHeader != NULL);
    assert(tableFile->freeBlocksHeader->first == -1);
    assert(tableFile->freeBlocksHeader->isDeleted == 0);
    assert(strcmp(tableFile->freeBlocksHeader->name, "freeBlocksHeader") == 0);
    free(tableFile);
}

void test_addFile()
{
    /* TableFile *tableFile = newTableFile();
    // !!! file must exist in memory
    char *name = "garabatos.txt";
    addFile(tableFile, name);
    assert(tableFile->filesCount == 1);
    assert(strcmp(tableFile->files[0]->name, name) == 0);

    free(tableFile); */
}

void test_serializeTableFile()
{
    TableFile *tableFile = newTableFile();
    tableFile->freeBlocksHeader->first = 150;
    char *filename = "test_serializeTableFile";
    serializeTableFile(tableFile, filename);
    free(tableFile);
}

void test_deserializeTableFile()
{
    char *filename = "test_serializeTableFile";
    TableFile *tableFile = deserializeTableFile(filename);
    assert(tableFile != NULL);
    free(tableFile);
}

int main()
{
    logLevel = 2;
    test_newTableFile();
    printf("test_newTableFile passed\n");

    test_addFile();
    printf("test_addFile passed\n");

    test_serializeTableFile();
    printf("test_serializeTableFile passed\n");

    test_deserializeTableFile();
    printf("test_deserializeTableFile passed\n");

    return 0;
}

// gcc -o tableFile_test tableFile_test.c tableFile.c file.c fileBlock.c verbose.c