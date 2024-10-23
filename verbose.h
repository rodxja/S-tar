#ifndef VERBOSE_H
#define VERBOSE_H

extern int logLevel; // 0 will not print anything, 1 will print basic infromation(errors), 2 will print all

void logAll(const char *format, ...);
void logError(const char *format, ...);
void logInfo(const char *format, ...);
void logDebug(const char *format, ...);

#endif // VERBOSE_H