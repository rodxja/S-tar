#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fileBlock.h"

void test_newFileBlock()
{
    FileBlock *fileBlock = newFileBlock();
    assert(fileBlock != NULL);
    assert(fileBlock->size == 0);
    assert(fileBlock->next == NULL);
    assert(fileBlock->data != NULL);
    free(fileBlock);
}

void test_setFileBlockData()
{
    FileBlock *fileBlock = newFileBlock();
    char data[BLOCK_SIZE];
    const int maxChar = 256;
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        char c = i % maxChar;
        data[i] = c;
    }
    setFileBlockData(fileBlock, data, BLOCK_SIZE);
    assert(fileBlock->size == BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        char c = i % maxChar;
        assert(fileBlock->data[i] == c);
    }
    free(fileBlock);
}

void test_serializeFileBlock()
{
    FileBlock *fileBlock = newFileBlock();
    char data[BLOCK_SIZE];
    const int maxChar = 256;
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        data[i] = 's';
    }
    setFileBlockData(fileBlock, data, BLOCK_SIZE);

    char *filename = "test_serializeFileBlock";
    FILE *file = fopen(filename, "w");
    serializeFileBlock(fileBlock, file);
    fclose(file);
    free(fileBlock);
}

void test_deserializeFileBlock()
{
    char *filename = "test_serializeFileBlock";
    FILE *file = fopen(filename, "r");
    FileBlock *fileBlock = deserializeFileBlock(file);
    fclose(file);
    assert(fileBlock != NULL);
    assert(fileBlock->size == BLOCK_SIZE);
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        assert(fileBlock->data[i] == 's');
    }
    free(fileBlock);
}

int main(int argc, char *argv[])
{
    test_newFileBlock();
    test_setFileBlockData();
    test_serializeFileBlock();
    test_deserializeFileBlock();
    printf("All tests passed\n");
    return 0;
}

// gcc -o fileBlock_test fileBlock_test.c fileBlock.c verbose.c