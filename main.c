#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "tableFile.h"

int main(int argc, char *argv[])
{
    printf("argc: %d\n", argc);
    // argv[0] will store the command name 'star'
    // argv[1] will store the option (e.g. -cvf)
    // argv[2] will store the output file name
    // argv[3:] will store the N input fileS name

    // first three are mandatory

    if (argc < 3)
    {
        // TODO : update usage
        printf("Usage: star <opciones> <archivoSalida>\n");
        return 1;
    }

    // Check if options are valid
    if (argv[1][0] != '-')
    {
        printf("Error: no valid option '%s'\n", argv[1]);
        return 1;
    }

    // Check if output file is valid
    if (argv[2] == NULL)
    {
        printf("Error: no valid output file\n");
        return 1;
    }

    TableFile *tableFile;

    for (int i = 1; i < strlen(argv[1]); i++)
    {
        switch (argv[1][i])
        {
        case 'c':
            tableFile = newTableFile();

            // if c is the only option, write the table file and exit
            writeTableFile(tableFile, argv[2]);
            break;

        case 'x':
            tableFile = loadTableFile(argv[2]);
            if (tableFile == NULL){
                printf("Error: no se pudo cargar el archivo %s\n", argv[2]);
                return 1;
            }
            extractFile(tableFile, argv[3]);
            break;

        // case 'f':
        //     // TODO : validate that there are files to add
        //     int areFiles = 0;
        //     for (int j = 3; j < argc; j++)
        //     {
        //         // TODO : validate that the file exists
        //         // TODO : validate that the file is not already in the table
        //         // TODO : validate that the file is not already in the free blocks
        //         // TODO : add the file to the table
        //         File *file = newFile(argv[j]);
        //         openFile(file);
        //         // TODO : ADD file to tableFile

        //         // TODO : read the file and add it to the table
        //         areFiles = 1;
        //     }
        //     if (!areFiles)
        //     {
        //         printf("Error: no files to add\n");
        //         return 1;
        //     }
        //     break;


        default:
            printf("Error: no valid option '%c' in '%s'\n", argv[1][i], argv[1]);
            return 1;
        }
    }

    return 0;
}