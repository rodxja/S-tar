#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <stdlib.h>

#include "file.h"

#define FILES_NUM 250

typedef struct
{
    // TODO : change to a dynamic array
    File *Files;
    File *FreeBlocks;
} TableFile;

TableFile *newTableFile();
void writeTableFile(TableFile *tableFile, char *outputFile);

#endif // TABLEINFO_H
