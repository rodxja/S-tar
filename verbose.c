#include <stdio.h>
#include <stdarg.h>

#include "verbose.h"

int logLevel = 0;
// 0 = just for logAll
// 1 = logAll and logError
// 2 = logInfo, logError and logInfo
// 3 = logDebug, logError, logInfo and logDebug

void logAll(const char *format, ...)
{
    va_list args;           // Declare a variable argument list
    va_start(args, format); // Initialize the list with the last fixed parameter

    vprintf(format, args); // Use vprintf to handle the variable arguments

    va_end(args); // Clean up the variable argument list
}

void logError(const char *format, ...)
{
    if (logLevel >= 1)
    {
        return;
    }

    va_list args;           // Declare a variable argument list
    va_start(args, format); // Initialize the list with the last fixed parameter

    vprintf(format, args); // Use vprintf to handle the variable arguments

    va_end(args); // Clean up the variable argument list
}

void logInfo(const char *format, ...)
{
    if (logLevel >= 2)
    {
        va_list args;           // Declare a variable argument list
        va_start(args, format); // Initialize the list with the last fixed parameter

        vprintf(format, args); // Use vprintf to handle the variable arguments

        va_end(args); // Clean up the variable argument list
    }
}

void logDebug(const char *format, ...)
{
    if (logLevel >= 3)
    {
        va_list args;           // Declare a variable argument list
        va_start(args, format); // Initialize the list with the last fixed parameter

        vprintf(format, args); // Use vprintf to handle the variable arguments

        va_end(args); // Clean up the variable argument list
    }
}