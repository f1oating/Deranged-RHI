//
// Created by alan on 12/08/2026.
//

#include "ShaderCompiler.h"
#include <cstring>
#include "Application.h"

#ifdef WIN32
#include <windows.h>
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;}
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
#endif

int main() {
    Application application;

    application.Run();

    return 0;
}