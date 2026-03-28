include(FetchContent)

# --- spdlog (logging) ---
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.0
    GIT_SHALLOW    TRUE
)

# --- nlohmann/json ---
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

# --- GLFW (windowing) ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# --- Dear ImGui (no native CMake, we build it ourselves) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8-docking
    GIT_SHALLOW    TRUE
)

# --- ImPlot ---
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v0.16
    GIT_SHALLOW    TRUE
)

# --- libserialport ---
FetchContent_Declare(
    libserialport
    GIT_REPOSITORY https://github.com/sigrokproject/libserialport.git
    GIT_TAG        libserialport-0.1.2
    GIT_SHALLOW    TRUE
)

# --- GoogleTest ---
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)

# Make dependencies available
FetchContent_MakeAvailable(spdlog json glfw imgui implot googletest)

# Fetch libserialport content but do not add_subdirectory (no CMakeLists.txt)
FetchContent_GetProperties(libserialport)
if(NOT libserialport_POPULATED)
    FetchContent_Populate(libserialport)
endif()

# --- Build Dear ImGui as a static library ---
find_package(OpenGL REQUIRED)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC
    glfw
    OpenGL::GL
)

# --- Build ImPlot as a static library ---
add_library(implot_lib STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(implot_lib PUBLIC
    ${implot_SOURCE_DIR}
)

target_link_libraries(implot_lib PUBLIC
    imgui_lib
)

# --- Build libserialport as a static library ---
set(LIBSERIALPORT_SOURCES
    ${libserialport_SOURCE_DIR}/serialport.c
    ${libserialport_SOURCE_DIR}/timing.c
)

if(WIN32)
    list(APPEND LIBSERIALPORT_SOURCES ${libserialport_SOURCE_DIR}/windows.c)
elseif(APPLE)
    list(APPEND LIBSERIALPORT_SOURCES ${libserialport_SOURCE_DIR}/macosx.c)
elseif(UNIX)
    list(APPEND LIBSERIALPORT_SOURCES ${libserialport_SOURCE_DIR}/linux.c ${libserialport_SOURCE_DIR}/linux_termios.c)
endif()

add_library(serialport STATIC ${LIBSERIALPORT_SOURCES})

target_include_directories(serialport PUBLIC
    ${libserialport_SOURCE_DIR}
)

if(MSVC)
    # Use MSBUILD path which defines SP_PRIV internally for the library build.
    # Also expose MSBUILD publicly so consumers get the correct SP_API linkage.
    target_compile_definitions(serialport PUBLIC LIBSERIALPORT_MSBUILD)
else()
    # For GCC/Clang, generate a minimal autoconf-style config.h
file(WRITE ${libserialport_SOURCE_DIR}/config.h
"#ifndef LIBSERIALPORT_CONFIG_H\n\
#define LIBSERIALPORT_CONFIG_H\n\
#define SP_PRIV __attribute__((visibility(\"hidden\")))\n\
#define SP_API  __attribute__((visibility(\"default\")))\n\
#define HAVE_TERMIOS_H 1\n\
#define HAVE_SYS_IOCTL_H 1\n\
#define HAVE_SYS_TIME_H 1\n\
#define HAVE_CLOCK_GETTIME 1\n\
#endif\n"
)
    target_compile_definitions(serialport PRIVATE LIBSERIALPORT_ATBUILD)
endif()

if(WIN32)
    target_link_libraries(serialport PUBLIC setupapi)
endif()
