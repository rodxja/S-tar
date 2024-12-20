#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "tableFile.h"
#include "verbose.h"
#include <unistd.h>

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

    TableFile *tableFile = NULL;
    char *options = argv[1];
    char *outputFile = argv[2];

    // TODO : first catch which actions to do, then do them
    for (int i = 1; i < strlen(options); i++)
    {
        switch (options[i])
        {
        case 'c':
            tableFile = newTableFile(outputFile);
            // if c is the only option, writes the table file and exit
            openTableFile(tableFile, "wb");
            create(tableFile, outputFile);

            break;
        case 'f':
            for (int j = 3; j < argc; j++)
            {
                char *fileName = argv[j];
                // add the file to the table
                addFile(tableFile, fileName);
                // areFiles = 1;
            }
            break;

        case 'v': // verbose
            logLevel++;
            break;

        case 'x':
            // validate that file exists

            tableFile = loadTableFile(outputFile);
            if (tableFile == NULL)
            {
                logError("Error: no se pudo cargar el archivo %s\n", outputFile);
                return 1;
            }
            openTableFile(tableFile, "rb");

            // TODO : pending to implement
            extractAllFiles(tableFile, ".");
            break;

        case 't':
            if (tableFile == NULL)
            {
               tableFile =  loadTableFile(outputFile);
            }
            if (tableFile->file == NULL)
            {
                openTableFile(tableFile, "rb");
            }
            listFiles(tableFile);
            break;

        case 'd':
            tableFile = loadTableFile(outputFile);
            openTableFile(tableFile, "rb+");
            // validate that file exists
            // validate that file is not already deleted
            // deleteFile(tableFile, fileName);
            for (int j = 3; j < argc; j++)
            {
                char *fileName = argv[j];

                delete (tableFile, fileName);
            }
            break;

        case 'r':
            tableFile = loadTableFile(outputFile);
            if (tableFile == NULL)
            {
                logError("Error: no se pudo cargar el archivo %s\n", outputFile);
                return 1;
            }
            openTableFile(tableFile, "rb+");

            for (int i = 3; i < argc; i++)
            {
                char *fileName = argv[i];
                addFile(tableFile, fileName);
            }
            break;

        case 'u':
            tableFile = loadTableFile(outputFile);
            if (tableFile == NULL)
            {
                logError("Error: no se pudo cargar el archivo %s\n", outputFile);
                return 1;
            }

            openTableFile(tableFile, "rb+");
            // validate that file exists

            for (int j = 3; j < argc; j++)
            {
                char *fileName = argv[j];

                update(tableFile, fileName);
            }
            break;

        case 'p':
            tableFile = loadTableFile(outputFile);
            openTableFile(tableFile, "rb+");
            pack(tableFile);
            break;

        default:
            logError("Error: no valid option '%c' in '%s'\n", options[i], options);
            return 1;
        }
    }

    closeTableFile(tableFile);

    return 0;
}
//  gcc -o star main.c tableFile.c verbose.c fileHeader.c fileBlock.c
// ./star -vvvcf test.star garabatos.txt otro.txt