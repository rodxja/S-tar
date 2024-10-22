#include <stdlib.h> // Include this header for malloc
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "fileBlock.h"
#include "verbose.h"

struct FileBlock *newFileBlock()
{
    FileBlock *fileBlock = (FileBlock *)malloc(sizeof(FileBlock));

    fileBlock->size = 0;
    fileBlock->next = NULL;

    return fileBlock;
}

void setFileBlockData(struct FileBlock *fileBlock, char data[BLOCK_SIZE], ssize_t bytesRead)
{
    if (fileBlock == NULL)
    {
        logError("Error: FileBlock is null for setFileBlockData\n");
        return;
    }

    if (data == NULL)
    {
        logError("Error: Data is null for setFileBlockData\n");
        return;
    }

    if (bytesRead < 0)
    {
        logError("Error: bytesRead is negative for setFileBlockData\n");
        return;
    }

    memcpy(fileBlock->data, data, bytesRead);
    fileBlock->size = bytesRead;
}

char *toJsonFileBlock(struct FileBlock *fileBlock)
{
    char *json = (char *)malloc(sizeof(char) * (fileBlock->size + 100));
    sprintf(json, "{\"size\": %ld, \"data\": \"", fileBlock->size);
    for (int i = 0; i < fileBlock->size; i++)
    {
        sprintf(json + i, "%c", fileBlock->data[i]);
    }
    sprintf(json + fileBlock->size, "\"}");
    return json;
}

// Function to serialize a FileBlock linked list
void serializeFileBlockList(FileBlock *head, FILE *file)
{
    while (head)
    {
        // Write the size of the data
        fwrite(&(head->size), sizeof(size_t), 1, file);
        // Write the data
        fwrite(head->data, sizeof(char), head->size, file);
        head = head->next;
    }
    size_t endMarker = 0; // Use 0 size to indicate the end of the list
    fwrite(&endMarker, sizeof(size_t), 1, file);
}

// Function to deserialize a FileBlock linked list
struct FileBlock *deserializeFileBlockList(FILE *file)
{
    FileBlock *head = NULL, *tail = NULL;
    size_t size;
    while (fread(&size, sizeof(size_t), 1, file) && size > 0)
    {
        FileBlock *newBlock = (FileBlock *)malloc(sizeof(FileBlock));
        newBlock->size = size;
        fread(newBlock->data, sizeof(char), size, file);
        newBlock->next = NULL;
        if (!head)
        {
            head = newBlock;
        }
        else
        {
            tail->next = newBlock;
        }
        tail = newBlock;
    }
    return head;
}