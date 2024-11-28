#include <stdio.h>

#include <platform/platform.h>

int main()
{
    b8            quitFlagged = FALSE;
    platformState state = {};
    if (platformStartup(&state))
    {
        printf("sucessfull platform startup\n");
        while (!quitFlagged)
        {
            quitFlagged = platformPumpMessages(&state);
        }
    }
    printf("shutting down\n");
    platformShutdown(&state);
}
