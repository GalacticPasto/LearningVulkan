#pragma once
#include "defines.h"

typedef struct platformState
{
    void *internalState;
} platformState;

DAPI b8   platformStartup(platformState *platformState);
DAPI void platformShutdown(platformState *platformState);
DAPI b8   platformPumpMessages(platformState *platformState);

void platformConsoleWriteError(const char *message, u8 level);
void platformConsoleWrite(const char *message, u8 level);
