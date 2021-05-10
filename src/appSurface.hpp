#pragma once

#include "header.hpp"

namespace mc {
    class AppSurface {
    private:
        struct {
#ifdef _WIN32
            HWND handle;
#endif // _WIN32
            mc::u32 width, height;
        } static s_;

    private:
#ifdef _WIN32
        static LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
            if (hwnd == mc::AppSurface::s_.handle) {
                switch (msg) {
                case WM_DESTROY:
                    mc::AppSurface::Release();
                    return 0;
                case WM_SIZE:
                    mc::AppSurface::s_.width  = GET_X_LPARAM(lParam);
                    mc::AppSurface::s_.height = GET_Y_LPARAM(lParam);
                    return 0;
                }
            }

            return DefWindowProcA(hwnd, msg, wParam, lParam);
        }
#endif // _WIN32

    public:
        static void Acquire() noexcept {
#ifdef _WIN32
            WNDCLASSA wc     = { };
            wc.lpfnWndProc   = mc::AppSurface::Win32WindowProc;
            wc.hInstance     = GetModuleHandleA(NULL);
            wc.lpszClassName = "Minecraft's Window Class";

            RegisterClassA(&wc);

            s_.handle = CreateWindowExA(0, "Minecraft's Window Class", "Minecraft - Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandleA(NULL), NULL);

            Show();
#endif // _WIN32
        }

#ifdef _WIN32
        static inline bool Exists() noexcept { return s_.handle != NULL; }
        static inline void Show()   noexcept { ShowWindow(s_.handle, SW_SHOW); }
        static inline void Hide()   noexcept { ShowWindow(s_.handle, SW_HIDE); }
        static inline void Free()   noexcept { DestroyWindow(s_.handle); s_.handle = NULL; }
#endif // _WIN32

        static inline u32 GetWidth()  noexcept { return s_.width; }
        static inline u32 GetHeight() noexcept { return s_.height; }

#ifdef _WIN32
        static inline HWND GetNativeHandle() noexcept { return s_.handle; }
#endif // _WIN32

        static void Update() noexcept {
#ifdef _WIN32
            MSG msg = { };
            while (PeekMessageA(&msg, s_.handle, 0, 0, PM_REMOVE) > 0) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
#endif // _WIN32
        }

        static void Release() noexcept {
            DestroyWindow(s_.handle);
            s_.handle = NULL;

            UnregisterClassA("Minecraft's Window Class", GetModuleHandleA(NULL));
        }
    }; // class AppSurface

#ifdef _WIN32
    decltype(AppSurface::s_) AppSurface::s_;
#endif // _WIN32

}; // namespace mc