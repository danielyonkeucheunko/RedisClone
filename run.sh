#!/bin/bash

# Exit immediately if a command fails
set -e

# Project root
PROJECT_ROOT=$(pwd)
BUILD_DIR="$PROJECT_ROOT/build"
BINARY_NAME="RedisClone"  # Must match the name in CMakeLists.txt

# Create build folder if it doesn't exist
mkdir -p "$BUILD_DIR"

# Configure CMake
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR"

# Build the project
cmake --build "$BUILD_DIR" --verbose

# Run the binary with any arguments passed to the script
# "$BUILD_DIR/$BINARY_NAME" "$@"

gnome-terminal -- bash -c "$BUILD_DIR/Server; exec bash"
gnome-terminal -- bash -c "$BUILD_DIR/Client; exec bash"
