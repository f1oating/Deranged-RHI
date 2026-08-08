file(GLOB_RECURSE SOURCES
    src/*.cpp
)

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

FetchContent_MakeAvailable(volk glfw)

add_library(RHI SHARED ${SOURCES})

target_include_directories(RHI
    PUBLIC
        include
    PRIVATE
        src
)

target_link_libraries(RHI
    PRIVATE
        vulkan
        volk
        glfw
)