#pragma once

#include "appSurface.hpp"
#include "renderer.hpp"

namespace mc {

    class Minecraft {
    public:
        Minecraft(int argc, char** argv) noexcept {
            AppSurface::Acquire();
            Renderer::Startup();
        }

        void Run() noexcept {
            while (AppSurface::Exists()) {
                AppSurface::Update();
                std::this_thread::yield();
            }
        }

        ~Minecraft() {
            AppSurface::Release();
            Renderer::Shutdown();
        }
    }; // class Minecraft

}; // namespace mc