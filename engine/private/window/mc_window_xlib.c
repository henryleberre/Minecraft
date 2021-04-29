#ifdef MC_DWM_XLIB

#include "public/misc/mc_error.h"

#include "public/window/mc_window.h"

bool mc_window_create (const mc_window_create_info_t* const pCreateInfo, mc_window_handle_t* const pWindowHandle) {
    return MC_FAIL_VALUE;
}

bool mc_window_show   (const mc_window_handle_t pWindowHandle) { return MC_FAIL_VALUE; }
bool mc_window_hide   (const mc_window_handle_t pWindowHandle) { return MC_FAIL_VALUE; }
bool mc_window_update (const mc_window_handle_t pWindowHandle) { return MC_FAIL_VALUE; }
bool mc_window_is_open(const mc_window_handle_t pWindowHandle) { return MC_FAIL_VALUE; }
bool mc_window_destroy(const mc_window_handle_t pWindowHandle) { return MC_FAIL_VALUE; }

#endif // MC_DWM_XLIB