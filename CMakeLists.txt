cmake_minimum_required(VERSION 3.22)
project(embview VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(cmake/Dependencies.cmake)

add_subdirectory(src)

option(EMBVIEW_BUILD_TESTS "Build unit tests" ON)
if(EMBVIEW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
