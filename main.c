#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "tableFile.h"
#include "verbose.h"

int main(int argc, char *argv[])
{
    logAll("argc: %d\n", argc);
    // argv[0] will store the command name 'star'
    // argv[1] will store the option (e.g. -cvf)
    // argv[2] will store the output file name
    // argv[3:] will store the N input fileS name

    // first three are mandatory

    if (argc < 3)
    {
        // TODO : update usage
        logAll("Usage: star <opciones> <archivoSalida>\n");
        return 1;
    }

    // Check if options are valid
    if (argv[1][0] != '-')
    {
        logAll("Error: no valid option '%s'\n", argv[1]);
        return 1;
    }

    // Check if output file is valid
    if (argv[2] == NULL)
    {
        logAll("Error: no valid output file\n");
        return 1;
    }

    TableFile *tableFile;

    // TODO : first catch which actions to do, then do them
    for (int i = 1; i < strlen(argv[1]); i++)
    {
        switch (argv[1][i])
        {
        case 'c':
            tableFile = newTableFile();

            // if c is the only option, write the table file and exit

            break;
        case 'f':
            // TODO : validate that there are files to add
            int areFiles = 0;
            for (int j = 3; j < argc; j++)
            {
                char *fileName = argv[j];
                // TODO : validate that the file exists in memory

                // TODO : validate that the file is not already in the table
                if (fileExists(tableFile, fileName))
                {
                    logError("Error: file '%s' already exists\n", fileName);
                    return 1;
                }
                // TODO : validate that the file is not already in the free blocks

                // add the file to the table
                addFile(tableFile, fileName);
                areFiles = 1;
            }
            if (!areFiles)
            {
                logError("Error: no files to add\n");
                return 1;
            }
            break;

        case 'v': // verbose
            logLevel++;
            break;

        case 'x':
            tableFile = loadTableFile(argv[2]);
            if (tableFile == NULL)
            {
                logError("Error: no se pudo cargar el archivo %s\n", argv[2]);
                return 1;
            }
            extractFile(tableFile, ".");
            break;
        
        case 't':
            tableFile = loadTableFile(argv[2]);
            if (tableFile == NULL)
            {
                logError("Error: no se pudo cargar el archivo %s\n", argv[2]);
                return 1;
            }
            listFiles(tableFile);
            break;

        default:
            logError("Error: no valid option '%c' in '%s'\n", argv[1][i], argv[1]);
            return 1;
        }
    }

    // TODO : this must be done if the option specifies it
    writeTableFile(tableFile, argv[2]);

    return 0;
}
// gcc -o star main.c tableFile.c file.c fileBlock.c verbose.c
// ./star -vvvcf test.star garabatos.txt otro.txt