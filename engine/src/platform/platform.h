#include "defines.h"

typedef struct platformState
{
    void *internalState;
} platformState;

DAPI b8   platformStartup(platformState *platformState);
DAPI void platformShutdown(platformState *platformState);
DAPI b8   platformPumpMessages(platformState *platformState);
