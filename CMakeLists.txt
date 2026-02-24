# =============================================================================
# CMakeLists.txt — 3D Library Simulation
# =============================================================================
# Build system for the 3D Library project using Modern OpenGL 3.3 and C++17.
# Supports both system-installed GLFW and a local copy in external/glfw/.
# =============================================================================

cmake_minimum_required(VERSION 3.16)
project(3DLibrary LANGUAGES C CXX)

# ---- C++ Standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # Use -std=c++17, not gnu++17

# ---- Collect Source Files ----
# All .cpp files that make up the application
set(SOURCES
    src/main.cpp
    src/core/Window.cpp
    src/core/Shader.cpp
    src/core/Camera.cpp
    src/renderer/Mesh.cpp
    external/glad/glad.c               # GLAD OpenGL loader (compiled as C)
)

# All header files (listed for IDE visibility, not strictly needed by CMake)
set(HEADERS
    src/core/Window.h
    src/core/Shader.h
    src/core/Camera.h
    src/renderer/Mesh.h
    src/renderer/Renderer.h
    src/utils/Logger.h
    src/utils/FileUtils.h
)

# ---- Create Executable ----
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ---- Include Directories ----
# src/     — so we can #include "core/Window.h" etc.
# external/ — for glad, glm, stb, KHR headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
)

# ---- Find OpenGL ----
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# ---- Find GLFW ----
# Try system-installed GLFW first; fall back to external/glfw/ if not found
find_package(glfw3 3.3 QUIET)

if(glfw3_FOUND)
    message(STATUS "Found system GLFW3")
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
else()
    # Look for a local GLFW build in external/glfw/
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/glfw/CMakeLists.txt")
        message(STATUS "Using local GLFW from external/glfw/")
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)
        target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
    else()
        message(FATAL_ERROR
            "GLFW3 not found! Install it system-wide or place it in external/glfw/")
    endif()
endif()

# ---- Platform-Specific Libraries ----
if(WIN32)
    # Windows needs no extra libs beyond OpenGL + GLFW (linked above)
elseif(UNIX AND NOT APPLE)
    # Linux typically needs these for windowing
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread X11)
elseif(APPLE)
    # macOS frameworks
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
endif()

# ---- Compiler Warnings ----
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- Output Info ----
message(STATUS "Building ${PROJECT_NAME} with C++${CMAKE_CXX_STANDARD}")
