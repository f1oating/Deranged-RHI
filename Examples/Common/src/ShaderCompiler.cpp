//
// Created by alan on 12/08/2026.
//

#include "ShaderCompiler.h"

#include <cstdio>
#include <cstring>

slang::IGlobalSession* ShaderCompiler::m_GlobalSession = nullptr;

void ShaderCompiler::Init() {
    slang::createGlobalSession(&m_GlobalSession);
}

void ShaderCompiler::Shutdown() {
    if (m_GlobalSession) {
        m_GlobalSession->release();
    }
}

std::vector<uint8_t> ShaderCompiler::CompileShader(const char* path) {
    slang::TargetDesc targetDesc = {
        .format = SLANG_SPIRV,
        .profile = m_GlobalSession->findProfile("spirv_1_5")
    };

    slang::SessionDesc sessionDesc = {
        .targets = &targetDesc,
        .targetCount = 1
    };

    slang::ISession* session = nullptr;
    m_GlobalSession->createSession(sessionDesc, &session);

    slang::IModule* module;
    {
        slang::IBlob* diagnosticBlob = nullptr;
        module = session->loadModule(path, &diagnosticBlob);
        if (diagnosticBlob) {
            printf("%s\n", (const char*)diagnosticBlob->getBufferPointer());
            diagnosticBlob->release();
        }
    }

    slang::IEntryPoint* entryPoint = nullptr;
    module->findEntryPointByName("main", &entryPoint);

    slang::IComponentType* componentTypes[] = {
        module, entryPoint
    };
    slang::IComponentType* composedProgram = nullptr;
    {
        slang::IBlob* diagnosticBlob = nullptr;
        session->createCompositeComponentType(componentTypes, 2, &composedProgram, &diagnosticBlob);
        if (diagnosticBlob) {
            printf("%s\n", (const char*)diagnosticBlob->getBufferPointer());
            diagnosticBlob->release();
        }
    }

    slang::IComponentType* linkedProgram = nullptr;
    composedProgram->link(&linkedProgram, nullptr);

    slang::IBlob* spirv = nullptr;
    {
        slang::IBlob* diagnosticBlob = nullptr;
        linkedProgram->getEntryPointCode(0, 0, &spirv, &diagnosticBlob);
        if (diagnosticBlob) {
            printf("%s\n", (const char*)diagnosticBlob->getBufferPointer());
            diagnosticBlob->release();
        }
    }

    std::vector<uint8_t> output;
    output.resize(spirv->getBufferSize());
    memcpy(output.data(), spirv->getBufferPointer(), output.size());

    spirv->release();
    linkedProgram->release();
    composedProgram->release();
    entryPoint->release();
    module->release();

    return output;
}