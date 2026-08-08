cmake_minimum_required(VERSION 4.3)
project(Deranged_RHI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_subdirectory(RHI)

add_executable(Deranged_RHI main.cpp)

target_link_libraries(Deranged_RHI
    PRIVATE
        RHI
)