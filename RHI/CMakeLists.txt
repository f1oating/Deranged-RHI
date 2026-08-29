file(GLOB SOURCES_CORE
    src/*.cpp
)

file(GLOB_RECURSE SOURCES_VULKAN
    src/Backend/Vulkan/*.cpp
)

if(WIN32)
    file(GLOB_RECURSE SOURCES_DX12
        src/Backend/DX12/*.cpp
    )
endif()

include(FetchContent)

find_package(Vulkan REQUIRED)

FetchContent_Declare(
        volk
        URL https://github.com/zeux/volk/archive/refs/tags/1.4.350.zip
        SOURCE_DIR ${CMAKE_BINARY_DIR}/volk
)

FetchContent_Declare(
        glfw
        URL https://github.com/glfw/glfw/releases/download/3.5.1/glfw-3.5.1.zip
        SOURCE_DIR ${CMAKE_BINARY_DIR}/glfw
)

if(WIN32)
    FetchContent_Declare(
        agility
        URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.619.5
        SOURCE_DIR ${CMAKE_BINARY_DIR}/agility
    )
    FetchContent_Declare(
        dxc
        URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2607/dxc_2026_07_29.zip
        SOURCE_DIR ${CMAKE_BINARY_DIR}/dxc
    )
    FetchContent_MakeAvailable(agility dxc)

    add_library(agility INTERFACE)
    target_include_directories(agility
        INTERFACE
            ${CMAKE_BINARY_DIR}/agility/build/native/include
    )

    add_library(dxc SHARED IMPORTED)
    target_include_directories(dxc INTERFACE ${CMAKE_BINARY_DIR}/dxc/inc)
    set_target_properties(dxc PROPERTIES
        IMPORTED_IMPLIB ${CMAKE_BINARY_DIR}/dxc/lib/x64/dxcompiler.lib
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/dxc/bin/x64/dxcompiler.dll
    )
endif()

FetchContent_MakeAvailable(volk glfw)
add_library(RHI SHARED ${SOURCES_CORE} ${SOURCES_VULKAN} ${SOURCES_DX12})

target_include_directories(RHI
    PUBLIC
        include
    PRIVATE
        src
)

target_link_libraries(RHI
    PRIVATE
        volk
        Vulkan::Vulkan
        glfw
        $<$<PLATFORM_ID:Windows>:agility>
        $<$<PLATFORM_ID:Windows>:d3d12>
        $<$<PLATFORM_ID:Windows>:dxgi>
        $<$<PLATFORM_ID:Windows>:dxguid>
        $<$<PLATFORM_ID:Windows>:d3dcompiler>
        $<$<PLATFORM_ID:Windows>:dxc>
)

if(WIN32)
    add_custom_command(TARGET RHI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/agility/build/native/bin/x64/D3D12Core.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/agility/build/native/bin/x64/d3d12SDKLayers.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxc/bin/x64/dxcompiler.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxc/bin/x64/dxil.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()