#include <engine/public/window/mc_window.h>

int main(int argc, char** argv) {
    mc_window_create_info_t windowCreateInfo;
    windowCreateInfo.innerWidth = 1920;
    windowCreateInfo.innerHeight = 1080;
    windowCreateInfo.title = "Test Window";

    mc_window_handle_t windowHandle;

    mc_window_create(&windowCreateInfo, &windowHandle);
    mc_window_show(windowHandle);



    mc_window_destroy(windowHandle);
}