#!/usr/bin/env sh

# Check for the OS type
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows with MinGW
    cmake ./src -B build -G "MinGW Makefiles"
else
    # Unix-based system (macOS/Linux)
    cmake ./src -B build -G
fi
cmake --build build
build/wrangler
