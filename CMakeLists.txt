cmake_minimum_required(VERSION 3.16)
project(TimeMosaic LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── FetchContent ────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ── Dear ImGui library (hand-rolled target) ─────────────────────
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
target_link_libraries(imgui_lib PUBLIC glfw)

# ── OpenGL ──────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# ── TinyEXIF ────────────────────────────────────────────────────
add_library(tinyexif STATIC
    ${CMAKE_SOURCE_DIR}/third_party/TinyEXIF/TinyEXIF.cpp
)
target_include_directories(tinyexif PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/TinyEXIF
)

# ── stb (header-only, implementation compiled once) ─────────────
add_library(stb_impl STATIC
    ${CMAKE_SOURCE_DIR}/src/stb_impl.cpp
)
target_include_directories(stb_impl PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/stb
)

# ── Application ─────────────────────────────────────────────────
file(GLOB APP_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM APP_SOURCES ${CMAKE_SOURCE_DIR}/src/stb_impl.cpp)

add_executable(TimeMosaic ${APP_SOURCES})

target_include_directories(TimeMosaic PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/stb
    ${CMAKE_SOURCE_DIR}/third_party/TinyEXIF
)

target_link_libraries(TimeMosaic PRIVATE
    imgui_lib
    OpenGL::GL
    tinyexif
    stb_impl
)

# ── Platform specifics ──────────────────────────────────────────
if(APPLE)
    target_link_libraries(TimeMosaic PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(TimeMosaic PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )
endif()
