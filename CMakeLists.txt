cmake_minimum_required(VERSION 4.3)
project(Deranged_RHI)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(RHI)

add_executable(Deranged_RHI main.cpp)

target_link_libraries(Deranged_RHI
    PRIVATE
        RHI
)