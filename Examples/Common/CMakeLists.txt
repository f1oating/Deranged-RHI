file(GLOB_RECURSE SOURCES
    src/*.cpp
)

include(FetchContent)

if(WIN32)
    set(SLANG_URL https://github.com/shader-slang/slang/releases/download/v2026.14.1/slang-2026.14.1-windows-x86_64.zip)
    set(SLANG_TARGET_PROPERTIES
        IMPORTED_IMPLIB ${CMAKE_BINARY_DIR}/slang/lib/slang.lib
    )
else()
    set(SLANG_URL https://github.com/shader-slang/slang/releases/download/v2026.14.1/slang-2026.14.1-linux-x86_64.zip)
    set(SLANG_TARGET_PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/slang/lib/libslang.so
    )
endif()

FetchContent_Declare(
    slang
    URL ${SLANG_URL}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/slang
)

FetchContent_MakeAvailable(slang)

add_library(slang SHARED IMPORTED)
set_target_properties(slang PROPERTIES
    ${SLANG_TARGET_PROPERTIES}
)

target_include_directories(slang INTERFACE ${CMAKE_BINARY_DIR}/slang/include)

if(WIN32)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/slang/bin/slang.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/slang/bin/slang-compiler.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()

add_library(Common STATIC ${SOURCES})

target_include_directories(Common
    PUBLIC
        include
    PRIVATE
        src
)

target_link_libraries(Common
    PUBLIC
        RHI
        slang
)