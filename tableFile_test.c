#include <assert.h>
#include <string.h>

#include "tableFile.h"
#include "verbose.h"

void test_newTableFile()
{
    TableFile *tableFile = newTableFile();
    assert(tableFile != NULL);
    assert(tableFile->filesCount == 0);
    assert(tableFile->freeBlocks != NULL);
    free(tableFile);
}

void test_addFile()
{
    TableFile *tableFile = newTableFile();
    // !!! file must exist in memory
    char *name = "garabatos.txt";
    addFile(tableFile, name);
    assert(tableFile->filesCount == 1);
    assert(strcmp(tableFile->files[0]->name, name) == 0);

    free(tableFile);
}

void test_serializeTableFile()
{
    TableFile *tableFile = newTableFile();
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

void test_fullSerializeDeserialize()
{
    TableFile *tableFile = newTableFile();
    char *name = "garabatos.txt";
    addFile(tableFile, name);
    char *filename = "test_fullSerializeDeserialize";
    serializeTableFile(tableFile, filename);
    TableFile *tableFile2 = deserializeTableFile(filename);
    assert(strcmp(tableFile->files[0]->name, tableFile2->files[0]->name) == 0);
    free(tableFile);
    free(tableFile2);
}

int main()
{
    logLevel = 4;
    test_newTableFile();
    printf("test_newTableFile passed\n");

    test_addFile();
    printf("test_addFile passed\n");

    test_serializeTableFile();
    printf("test_serializeTableFile passed\n");

    test_deserializeTableFile();
    printf("test_deserializeTableFile passed\n");

    test_fullSerializeDeserialize();
    printf("test_fullSerializeDeserialize passed\n");

    return 0;
}