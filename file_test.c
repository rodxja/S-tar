#include <assert.h>
#include <string.h>

#include "file.h"

void test_newFile()
{
    File *file = newFile();
    assert(file != NULL);
    assert(file->name == NULL);
    assert(file->head == NULL);
    assert(file->tail == NULL);
    free(file);
}

void test_setNameFile()
{
    File *file = newFile();
    char *name = "garabatos.pdf";
    setNameFile(file, name);
    assert(file->name != NULL);
    assert(strcmp(file->name, name) == 0);
    free(file->name);
    free(file);
}

void test_addOneBlock()
{
    File *file = newFile();
    FileBlock *block = newFileBlock();
    char data[BLOCK_SIZE];
    int maxChar = 256;
    for (int i = 0; i < maxChar; i++)
    {
        data[i] = 's';
    }
    setFileBlockData(block, data, maxChar);
    addBlock(file, block);
    assert(file->head == block);
    assert(file->tail == block);

    // check content of node
    assert(file->head->size == maxChar);
    for (int i = 0; i < maxChar; i++)
    {
        assert(file->head->data[i] == 's');
    }
    free(file);
}

void test_addTwoBlocks()
{
    File *file = newFile();
    FileBlock *block = newFileBlock();
    char data[BLOCK_SIZE];
    int maxChar = 256;
    for (int i = 0; i < maxChar; i++)
    {
        data[i] = 's';
    }
    setFileBlockData(block, data, maxChar);
    addBlock(file, block);
    assert(file->head == block);
    assert(file->tail == block);

    // check content of node
    assert(file->head->size == maxChar);
    for (int i = 0; i < maxChar; i++)
    {
        assert(file->head->data[i] == 's');
    }

    // add a second block
    FileBlock *block2 = newFileBlock();
    for (int i = 0; i < maxChar; i++)
    {
        data[i] = 't';
    }
    setFileBlockData(block2, data, maxChar);
    addBlock(file, block2);
    assert(file->head == block);
    assert(file->tail == block2);

    // check content of node
    assert(file->head->size == maxChar);
    for (int i = 0; i < maxChar; i++)
    {
        assert(file->head->data[i] == 's');
    }

    // check content of node
    assert(file->tail->size == maxChar);
    for (int i = 0; i < maxChar; i++)
    {
        assert(file->tail->data[i] == 't');
    }
    free(file);
    free(block);
    free(block2);
}

void test_serializeFile()
{
    File *file = newFile();
    file->isDeleted = 1;
    char *name = "garabatos.pdf";
    setNameFile(file, name);
    FileBlock *block = newFileBlock();
    char data[BLOCK_SIZE];
    int maxChar = BLOCK_SIZE;
    for (int i = 0; i < maxChar; i++)
    {
        data[i] = 's';
    }
    setFileBlockData(block, data, maxChar);
    addBlock(file, block);

    FileBlock *block2 = newFileBlock();
    int maxChar2 = 256;
    for (int i = 0; i < maxChar2; i++)
    {
        data[i] = 't';
    }
    setFileBlockData(block2, data, maxChar2);
    addBlock(file, block2);

    char *filename = "test_serializeFile";
    FILE *f = fopen(filename, "w");
    serializeFile(file, f);
    fclose(f);
    free(file);
}

void test_deserializeFile()
{
    char *filename = "test_serializeFile";
    FILE *f = fopen(filename, "r");
    File *file = deserializeFile(f);
    fclose(f);
    free(file);
}

void test_fullSerializeDeserialize()
{
    File *file = newFile();
    file->isDeleted = 1;
    char *name = "garabatos.pdf";
    setNameFile(file, name);
    FileBlock *block = newFileBlock();
    char data[BLOCK_SIZE];
    int maxChar = BLOCK_SIZE;
    for (int i = 0; i < maxChar; i++)
    {
        data[i] = 's';
    }
    setFileBlockData(block, data, maxChar);
    addBlock(file, block);

    FileBlock *block2 = newFileBlock();
    int maxChar2 = 256;
    for (int i = 0; i < maxChar2; i++)
    {
        data[i] = 't';
    }
    setFileBlockData(block2, data, maxChar2);
    addBlock(file, block2);

    char *filename = "test_fullSerializeDeserialize";
    FILE *f = fopen(filename, "w");
    serializeFile(file, f);
    fclose(f);

    f = fopen(filename, "r");
    File *file2 = deserializeFile(f);
    fclose(f);

    assert(strcmp(file->name, file2->name) == 0);
    assert(file->head->size == file2->head->size);
    // data 
    for (int i = 0; i < file->head->size; i++)
    {
        assert(file->head->data[i] == file2->head->data[i]);
    }
    assert(file->tail->size == file2->tail->size);
    // data
    for (int i = 0; i < file->tail->size; i++)
    {
        assert(file->tail->data[i] == file2->tail->data[i]);
    }
    assert(file->isDeleted == file2->isDeleted);

    free(file);
    free(file2);
}

int main()
{
    test_newFile();
    printf("test_newFile passed\n");

    test_setNameFile();
    printf("test_setNameFile passed\n");

    test_addOneBlock();
    printf("test_addOneBlock passed\n");

    test_addTwoBlocks();
    printf("test_addTwoBlocks passed\n");

    test_serializeFile();
    printf("test_serializeFile passed\n");

    test_deserializeFile();
    printf("test_deserializeFile passed\n");

    test_fullSerializeDeserialize();
    printf("test_fullSerializeDeserialize passed\n");

    return 0;
}

// gcc -o file_test file_test.c file.c fileBlock.c verbose.c