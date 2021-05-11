#include "minecraft.hpp"

int main(int argc, char** argv) {
    try {
        mc::Minecraft::Startup(argc, argv);
        mc::Minecraft::Run();
        mc::Minecraft::Terminate();
    } catch (const std::runtime_error& e) {
        std::cout << e.what() << '\n';
    }
}