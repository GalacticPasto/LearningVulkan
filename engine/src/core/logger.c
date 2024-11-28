#include "logger.h"
#include "platform/platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

b8 initializeLogging()
{
    // TODO : create log file
    return TRUE;
}

void shutdownLogging()
{
    // TODO : clanup logging/write queued entries
}

void logOutput(logLevel level, const char *message, ...)
{
    const char *levelStrings[6] = {"[FATAL:]", "[ERROR:]", "[WARN:]", "[INFO:]", "[DEBUG:]", "[TRACE:]"};

    b8 isError = level < LOG_LEVEL_WARN;

    const i32 msgSize = 32000;
    char      outMessage[msgSize];
    memset(outMessage, 0, sizeof(outMessage));

    va_list argPtr;

    va_start(argPtr, message);
    vsnprintf(outMessage, msgSize, message, argPtr);
    va_end(argPtr);

    char outMessage2[msgSize];
    sprintf(outMessage2, "%s%s\n", levelStrings[level], outMessage);

    if (isError)
    {
        platformConsoleWriteError(outMessage2, level);
    }
    else
    {
        platformConsoleWrite(outMessage2, level);
    }
}

void reportAssertionFailure(const char *expression, const char *message, const char *file, i32 line)
{
    logOutput(LOG_LEVEL_FATAL, "AssertionFailure : %s message : %s,in File: %s, line: %d\n", expression, message, file,
              line);
}
