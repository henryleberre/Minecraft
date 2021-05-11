#pragma once

#include "appSurface.hpp"
#include "renderer.hpp"

namespace mc {

    class Minecraft {
    public:
        static void Startup(int argc, char** argv) {
            AppSurface::Acquire();
            Renderer::Startup();
        }

        static void Update() noexcept {

        }

        static void Render() noexcept {
            Renderer::Render();
        }

        static void Run() noexcept {
            while (AppSurface::Exists()) {
                AppSurface::Update();
                Minecraft::Update();
                Minecraft::Render();

                std::this_thread::yield();
            }
        }

        static void Terminate() noexcept {
            AppSurface::Release();
            Renderer::Shutdown();
        }
    }; // class Minecraft

}; // namespace mc