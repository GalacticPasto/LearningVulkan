#include "platform/platform.h"

#ifdef DPLATFORM_WINDOWS

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef struct internalState
{
    HINSTANCE hInstance;
    HWND      hWindow;
} internalState;

static b8 quitFlagged = FALSE;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

b8 platformStartup(platformState *platformState)
{
    platformState->internalState = malloc(sizeof(internalState));
    internalState *state = (internalState *)platformState->internalState;

    state->hInstance = GetModuleHandleA(NULL);

    char *className = "Learning Vulkan";
    HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);

    WNDCLASSA window = {};
    window.style = CS_HREDRAW | CS_OWNDC | CS_VREDRAW;
    window.lpfnWndProc = WindowProc;
    window.hInstance = state->hInstance;
    window.hIcon = icon;
    window.hCursor = LoadCursor(NULL, IDC_ARROW);
    window.lpszClassName = className;

    if (!RegisterClassA(&window))
    {
        printf("Cannot register Window\n");
        return FALSE;
    }

    u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 windowExStyle = WS_EX_APPWINDOW;
    windowStyle |= WS_MAXIMIZEBOX;
    windowStyle |= WS_MINIMIZEBOX;
    windowStyle |= WS_THICKFRAME;

    HWND hWindow = CreateWindowEx(windowExStyle, className, "Learning vulkan", windowStyle,
                                  // size and position
                                  0, 0, 1280, 720, 0, 0, state->hInstance, 0);

    if (hWindow == NULL)
    {
        printf("Cannot create windows :(\n");
        return FALSE;
    }
    state->hWindow = hWindow;

    ShowWindow(hWindow, SW_SHOW);
    return TRUE;
}

void platformShutdown(platformState *platformState)
{
    printf("shutinggg down ....\n");
}

b8 platformPumpMessages(platformState *platformState)
{
    MSG message;

    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
        if (quitFlagged)
        {
            printf("quit flagged recieved \n");
            return FALSE;
        }
    }
    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
        case WM_CLOSE: {
            quitFlagged = TRUE;
            return 0;
        }
        break;
        default: {
        }
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif
