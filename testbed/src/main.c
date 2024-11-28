#include <stdio.h>

#include <core/logger.h>
#include <platform/platform.h>

int main()
{

    DFATAL("its workind %f", 3.12313131);
    DERROR("its workind %f", 3.12313131);
    DWARN("its workind %f", 3.12313131);
    DDEBUG("its workind %f", 3.12313131);
    DTRACE("its workind %f", 3.12313131);

    b8            quitFlagged = TRUE;
    platformState state = {};
    if (platformStartup(&state))
    {
        printf("sucessfull platform startup\n");
        while (quitFlagged)
        {
            quitFlagged = platformPumpMessages(&state);
        }
    }
    printf("shutting down\n");
    platformShutdown(&state);
}
