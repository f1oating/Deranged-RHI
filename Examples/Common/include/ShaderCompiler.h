//
// Created by alan on 12/08/2026.
//

#ifndef DERANGED_RHI_SHADERCOMPILER_H
#define DERANGED_RHI_SHADERCOMPILER_H

#include <vector>
#include <slang.h>

class ShaderCompiler {
public:
    static void Init();
    static void Shutdown();

    static std::vector<uint8_t> CompileShader(const char* path);

private:
    static slang::IGlobalSession* m_GlobalSession;

};

#endif //DERANGED_RHI_SHADERCOMPILER_H
