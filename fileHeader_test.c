
#include <assert.h>
#include <stdlib.h>
#include <stdio.h> // Add this line to include the header file for printf
#include <string.h>

#include "fileHeader.h"

void test_newFileHeader()
{
    FileHeader *fileHeader = newFileHeader();
    assert(fileHeader != NULL);
    assert(fileHeader->first == 0);
    free(fileHeader);
}

void test_setNameFileHeader()
{
    FileHeader *fileHeader = newFileHeader();
    char *name = "garabatos.pdf";
    setNameFileHeader(fileHeader, name);
    assert(strcmp(fileHeader->name, name) == 0);
    free(fileHeader);
}

void test_serializeFileHeader()
{
    FileHeader *fileHeader = newFileHeader();
    char *name = "garabatos.pdf";
    setNameFileHeader(fileHeader, name);
    fileHeader->first = 70;
    char *filename = "test_serializeFileHeader";
    FILE *file = fopen(filename, "w");
    serializeFileHeader(fileHeader, file);
    fclose(file);
    free(fileHeader);
}

void test_deserializeFileHeader()
{
    char *filename = "test_serializeFileHeader";
    FILE *file = fopen(filename, "r");
    FileHeader *fileHeader = deserializeFileHeader(file);
    assert(fileHeader != NULL);
    assert(strcmp(fileHeader->name, "garabatos.pdf") == 0);
    assert(fileHeader->first == 70);
    fclose(file);
    free(fileHeader);
}

int main()
{
    test_newFileHeader();
    printf("test_newFileHeader passed\n");

    test_setNameFileHeader();
    printf("test_setNameFileHeader passed\n");

    test_serializeFileHeader();
    printf("test_serializeFileHeader passed\n");

    test_deserializeFileHeader();
    printf("test_deserializeFileHeader passed\n");

    return 0;
}

// gcc -o fileHeader_test fileHeader_test.c fileHeader.c verbose.c