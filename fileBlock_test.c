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

void test_toJsonFileBlock()
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
    char *json = toJsonFileBlock(fileBlock);
    printf("json: %s\n", json);
    free(json);
    free(fileBlock);
}

int main(int argc, char *argv[])
{
    test_newFileBlock();
    test_setFileBlockData();
    test_toJsonFileBlock();
    printf("All tests passed\n");
    return 0;
}