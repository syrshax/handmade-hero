# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name
project(HandmadeHero)

# Set the C++ standard (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the PkgConfig module, which is used to find libraries
find_package(PkgConfig REQUIRED)

# --- This is the replacement for `pkg-config --cflags --libs sdl3` ---
# Check for the SDL3 library
pkg_check_modules(SDL3 REQUIRED sdl3)
# --------------------------------------------------------------------

# Define your executable
# This tells CMake to create "handmadehero" from the source file
add_executable(handmadehero src/sdl_handmade.cpp)

# --- This replaces the --cflags part ---
# Add the SDL3 include directories to your target
target_include_directories(handmadehero PUBLIC ${SDL3_INCLUDE_DIRS})

# --- This replaces the --libs part ---
# Link the SDL3 libraries to your target
target_link_libraries(handmadehero PUBLIC ${SDL3_LIBRARIES})

# --- This replaces the -g flag ---
# This line ensures that a "Debug" build is the default,
# which automatically adds the -g flag for debugging.
# CLion manages this for you, but it's a good default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
endif()