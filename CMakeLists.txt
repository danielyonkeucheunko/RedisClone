cmake_minimum_required(VERSION 3.22)
project(RedisClone VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable strict warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Weffc++
        -Wconversion
        -Werror
        -pedantic-errors
        -fdiagnostics-color=always
        -ggdb
    )
endif()

# Collect all source files recursively from src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(Server src/server.cpp src/buffer.cpp)
add_executable(Client src/client.cpp src/buffer.cpp)

# # Optionally include headers
# target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")
