#ifndef MC_WINDOW_WINDOW_H
#define MC_WINDOW_WINDOW_H

#include "../misc/mc_types.h"
#include "../misc/mc_error.h"

typedef void* mc_window_handle_t;

typedef struct {
    u32 innerWidth, innerHeight;
    
    const char* title;
} mc_window_create_info_t; // struct mc_window_create_info_t

bool mc_window_create (const mc_window_create_info_t* const pCreateInfo, mc_window_handle_t* const pWindowHandle);
bool mc_window_show   (const mc_window_handle_t windowHandle);
bool mc_window_hide   (const mc_window_handle_t windowHandle);
bool mc_window_update (const mc_window_handle_t windowHandle);
bool mc_window_is_open(const mc_window_handle_t windowHandle);
bool mc_window_destroy(const mc_window_handle_t windowHandle);

#endif // MC_WINDOW_WINDOW_H