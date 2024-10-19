#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <stdlib.h>

#include "file.h"

#define FILES_NUM 250

typedef struct
{
    File *Files[FILES_NUM];
    File *FreeBlocks;
} TableFile;

TableFile *newTableFile();
void writeTableFile(TableFile *tableFile, char *outputFile);

#endif // TABLEINFO_H
