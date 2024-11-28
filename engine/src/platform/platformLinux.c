#include "platform/platform.h"

#ifdef DPLATFORM_LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_util.h>

typedef struct internalstate
{
    xcb_connection_t *connection;
    xcb_screen_t *    screen;
    xcb_drawable_t    window;
    xcb_atom_t        wmProtocols;
    xcb_atom_t        wmDeleteWin;
} internalState;

b8 platformStartup(platformState *platformState)
{

    char *applicationName = "Linux Platform Layer";

    platformState->internalState = malloc(sizeof(internalState));
    internalState *state = (internalState *)platformState->internalState;

    xcb_gcontext_t foreground;
    xcb_gcontext_t background;

    i32 screenNum;
    state->connection = xcb_connect(NULL, &screenNum);

    if (xcb_connection_has_error(state->connection))
    {
        printf("failed to connect to X server via XCB");
        return FALSE;
    }

    // get screen setup
    const struct xcb_setup_t *screenSetup;

    // get the first screen
    // TODO: make this configurable
    state->screen = xcb_aux_get_screen(state->connection, screenNum);

    // root window
    state->window = xcb_generate_id(state->connection);

    // Register event types.
    // XCB_CW_BACK_PIXEL = filling then window bg with a single level
    // XCB_CW_EVENT_MASK is required.
    u32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    // Listen for keyboard and mouse buttons
    u32 eventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                      XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                      XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    // Values to be sent over XCB (bg level, events)
    u32 valueList[] = {state->screen->black_pixel, eventValues};

    // create window
    xcb_create_window(state->connection,             /* connection    */
                      XCB_COPY_FROM_PARENT,          /* depth         */
                      state->window,                 /* window Id     */
                      state->screen->root,           /* parent window */
                      0, 0,                          /* x, y          */
                      1280, 720,                     /* width, height */
                      0,                             /* border_width  */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class         */
                      state->screen->root_visual,    /* visual        */
                      eventMask, valueList);         /* masks         */

    xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE, state->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                        8, // data should be viewed 8 bits at a time
                        strlen(applicationName), applicationName);

    // Tell the server to notify when the window manager
    // attempts to destroy the window.
    xcb_intern_atom_cookie_t wmDeleteCookie =
        xcb_intern_atom(state->connection, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wmProtocolsCookie =
        xcb_intern_atom(state->connection, 1, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");

    xcb_intern_atom_reply_t *wmDeleteReply = xcb_intern_atom_reply(state->connection, wmDeleteCookie, NULL);
    xcb_intern_atom_reply_t *wmProtocolsReply = xcb_intern_atom_reply(state->connection, wmProtocolsCookie, NULL);

    state->wmDeleteWin = wmDeleteReply->atom;
    state->wmProtocols = wmProtocolsReply->atom;

    xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE, state->window, wmProtocolsReply->atom, 4, 32, 1,
                        &wmDeleteReply->atom);
    // map the window
    xcb_map_window(state->connection, state->window);

    i32 streamResult = xcb_flush(state->connection);

    if (streamResult <= 0)
    {
        printf("An error when flushing the XCB stream %d", streamResult);
        return FALSE;
    }
    return TRUE;
}

void platformShutdown(platformState *platformState)
{
    internalState *state = (internalState *)platformState->internalState;

    xcb_destroy_window(state->connection, state->window);
}

b8 platformPumpMessages(platformState *platformState)
{
    internalState *             state = (internalState *)platformState->internalState;
    xcb_generic_event_t *       event;
    xcb_client_message_event_t *cm;
    b8                          quitFlagged = FALSE;

    while ((event = xcb_poll_for_event(state->connection)))
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {
            }
            break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {
            }
            break;
            case XCB_MOTION_NOTIFY: {
            }
            break;
            case XCB_CONFIGURE_NOTIFY: {
            }
            break;
            case XCB_CLIENT_MESSAGE: {
                printf("recieved message");
                cm = (xcb_client_message_event_t *)event;
                if (cm->data.data32[0] == state->wmDeleteWin)
                {
                    quitFlagged = TRUE;
                }
            }
            break;
            default: {
            }
            break;
        }
        free(event);
    }
    return quitFlagged;
}

void platformConsoleWrite(const char *message, u8 level)
{
    const char *colourStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;35", "1;36"};
    printf("\033[%sm%s\033[0m", colourStrings[level], message);
    // printf("%s", message);
}

void platformConsoleWriteError(const char *message, u8 level)
{
    const char *colourStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;35", "1;36"};
    printf("\033[%sm%s\033[0m", colourStrings[level], message);
    // printf("%s", message);
}
#endif
