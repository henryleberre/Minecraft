#ifdef MC_DWM_XLIB

#include "public/misc/mc_error.h"
#include "public/misc/mc_includes.h"
#include "public/window/mc_window.h"

typedef struct {
    Display* pDisplay;
    Window   window;
} _mc_window_state_t; // struct _mc_window_state_t

bool mc_window_create (const mc_window_create_info_t* const pCreateInfo, mc_window_handle_t* const pWindowHandle) {
    (*pWindowHandle) = (mc_window_handle_t)malloc(sizeof(_mc_window_state_t));

    _mc_window_state_t* const pWindowState = (_mc_window_state_t*)(*pWindowHandle);

    pWindowState->pDisplay = XOpenDisplay(NULL);
    pWindowState->window   = XCreateSimpleWindow(pWindowState->pDisplay, DefaultRootWindow(pWindowState->pDisplay), 0, 0, pCreateInfo->innerWidth, pCreateInfo->innerHeight, 0, BlackPixel(pWindowState->pDisplay, 0), WhitePixel(pWindowState->pDisplay, 0));

	XSetStandardProperties(pWindowState->pDisplay, pWindowState->window, pCreateInfo->title, pCreateInfo->title, None, NULL, 0, NULL);

    return MC_SUCCESS_VALUE;
}

bool mc_window_show   (const mc_window_handle_t windowHandle) {
    const _mc_window_state_t* const pWindowState = (const _mc_window_state_t* const)windowHandle;

    XMapWindow(pWindowState->pDisplay, pWindowState->window);
	XFlush(pWindowState->pDisplay);

    return MC_SUCCESS_VALUE;
}

bool mc_window_hide   (const mc_window_handle_t windowHandle) {
    const _mc_window_state_t* const pWindowState = (const _mc_window_state_t* const)windowHandle;

    XUnmapWindow(pWindowState->pDisplay, pWindowState->window);
	XFlush(pWindowState->pDisplay);

    return MC_SUCCESS_VALUE;
}

bool mc_window_update (const mc_window_handle_t windowHandle) { return MC_FAIL_VALUE; }
bool mc_window_is_open(const mc_window_handle_t windowHandle) { return MC_FAIL_VALUE; }

bool mc_window_destroy(const mc_window_handle_t windowHandle) {
    const _mc_window_state_t* const pWindowState = (const _mc_window_state_t* const)windowHandle;

    XDestroyWindow(pWindowState->pDisplay, pWindowState->window);
	XCloseDisplay(pWindowState->pDisplay);

    free((void*)windowHandle);

    return MC_SUCCESS_VALUE;
}

#endif // MC_DWM_XLIB