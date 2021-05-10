#pragma once

/*
 * Project    : Minecraft
 * Author     : Henry LE BERRE (https://github.com/henryleberre)
 * Repository : https://github.com/henryleberre/Minecraft
 */

/*
 * It is here that we shall begin. Don't fret, I'll guide you.
 */

/*
 * System Library Includes
 */

#ifdef _WIN32
#   define NOMINMAX

#   include <Windows.h>
#   include <windowsx.h>

#   pragma comment(lib, "user32.lib")
#   pragma comment(lib, "kernel32.lib")

#   undef NOMINMAX
#endif // _WIN32

/*
 * STL <3: What would we do without std::chrono::time_point<std::chrono::high_resolution_clock> ?
 * Rust will never win... Please don't Rust. What's your deal with references ?
 * C is sheer clarity.
 */

#ifdef _WIN32
#   define _CRT_SECURE_NO_WARNINGS
#endif // _WIN32

#include <set>
#include <map>
#include <list>
#include <array>
#include <queue>
#include <mutex>
#include <cmath>
#include <future>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include <bitset>
#include <cassert>
#include <cstring>
#include <numeric>
#include <codecvt>
#include <variant>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <optional>
#include <exception>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#   undef _CRT_SECURE_NO_WARNINGS
#endif // _WIN32

/*
 * Vulkan Includes.
 * Vulkan, don't worry, I'll feed you pointers to structs.
 */

#ifdef _WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

#define VULKAN_HPP_NO_EXCEPTIONS

#include <vulkan/vulkan.hpp>

#undef VULKAN_HPP_NO_EXCEPTIONS

#ifdef _WIN32
#   pragma comment(lib, "vulkan-1.lib")

#   undef VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

/*
 * Nice Platform Detection Macros
 */

#ifdef _WIN32
#   define DO_WINDOWS(x) x
#   define DO_WINDOWS_COMMA(x) x,
#   define DO_LINUX(x)
#   define DO_LINUX_COMMA(x)
#   define DO_X11(x)
#   define DO_X11_COMMA(x)
#endif // _WIN32

#ifdef NDEBUG
#   define DO_RELEASE(x) x
#   define DO_RELEASE_COMMA(x) x,
#   define DO_DEBUG(x)
#   define DO_DEBUG_COMMA(x)
#else
#   define DO_RELEASE(x)
#   define DO_RELEASE_COMMA(x)
#   define DO_DEBUG(x) x
#   define DO_DEBUG_COMMA(x) x,
#endif

/*
 * Better basic typenames.
 */

namespace mc {
    using f32 = float;        using f64 = double;
    using i8  = std::int8_t;  using i16 = std::int16_t;  using i32 = std::int32_t;  using i64 = std::int64_t;
    using u8  = std::uint8_t; using u16 = std::uint16_t; using u32 = std::uint32_t; using u64 = std::uint64_t;
}; // namespace mc

/*
 * Constants...
 * They made life happen, let's be courteous with them, they don't like change
 */

namespace mc {
    constexpr const char* MC_APPLICATION_NAME    = "Minecraft - Client";
    constexpr u32         MC_APPLICATION_VERSION = VK_MAKE_VERSION(1, 0, 0, 0);

    constexpr const char* MC_ENGINE_NAME    = "F. Weiss <3";
    constexpr u32         MC_ENGINE_VERSION = MC_APPLICATION_VERSION;

    constexpr u32 MC_VULKAN_VERSION = VK_VERSION_1_0;

    constexpr inline std::array MC_VULKAN_INSTANCE_EXTENSIONS = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        DO_LINUX_COMMA(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
        DO_WINDOWS_COMMA(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
        DO_DEBUG_COMMA(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
    };

    constexpr inline std::array MC_VULKAN_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    constexpr inline std::array<const char*, 0> MC_VULKAN_LAYERS = { };
#else
    constexpr inline std::array MC_VULKAN_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };
#endif
}; // namespace mc
