#include "Device.h"

int main() {
    Device* device = Device::Create();
    Swapchain* swapchain = device->CreateSwapchain();

    while(!swapchain->WindowShouldClose()) {
        swapchain->UpdateWindow();
    }

    delete swapchain;
    delete device;
    return 0;
}
