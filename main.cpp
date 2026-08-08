#include "Device.h"

#ifdef WIN32
#include <windows.h>
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;}
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
#endif

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
