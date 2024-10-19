#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tableFile.h"

int main(int argc, char *argv[])
{

    // argv[0] will store the command name 'star'
    // argv[1] will store the option (e.g. -cvf)
    // argv[2] will store the output file name
    // argv[3:] will store the N input fileS name

    // first three are mandatory

    if (argc < 3)
    {
        // TODO : update usage
        printf("Usage: tar <opciones> <archivoSalida>\n");
        return 1;
    }

    // Check if options are valid
    if (argv[1][0] != '-')
    {
        printf("Error: Opcion invalida\n");
        return 1;
    }

    if (argv[1][1] != 'c')
    {
        printf("Error: Opcion invalida\n");
        return 1;
    }

    // Check if output file is valid
    if (argv[2] == NULL)
    {
        printf("Error: Archivo de salida invalido\n");
        return 1;
    }

    // TODO change this implementation
    TableFile *tableFile = newTableFile();
    writeTableFile(tableFile, argv[2]);

    return 0;
}