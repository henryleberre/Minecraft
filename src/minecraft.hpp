#pragma once

#include "appSurface.hpp"
#include "renderer.hpp"
#include "timer.hpp"

namespace mc {

    class Minecraft {
    public:
        static void Startup(int argc, char** argv) {
            AppSurface::Acquire();
            Renderer::Startup();
        }

        static void Update() {
            AppSurface::Update();
        }

        static void Render() {
            Renderer::Render();
        }

        static void Run() {
            while (AppSurface::Exists()) {
                Minecraft::Update();
                Minecraft::Render();

                std::this_thread::yield();
            }
        }

        static void Terminate() {
            AppSurface::Release();
            Renderer::Shutdown();
        }
    }; // class Minecraft

}; // namespace mc