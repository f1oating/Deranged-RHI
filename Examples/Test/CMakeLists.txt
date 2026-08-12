file(GLOB_RECURSE SOURCES
        src/*.cpp
)

add_executable(Test ${SOURCES})

target_include_directories(Test
    PRIVATE
        include
        src
)

target_link_libraries(Test
    PRIVATE
        Common
)