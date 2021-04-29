#ifndef MC_MISC_INCLUDES_H
#define MC_MISC_INCLUDES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#if defined(MC_OS_WINDOWS)
#   include <Windows.h>
#   include <windowsx.h>

#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(MC_DWM_XLIB)
#   include <X11/Xos.h>
#   include <X11/Xlib.h>
#   include <X11/Xutil.h>

#   define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

#endif // MC_MISC_INCLUDES_H