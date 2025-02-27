#!/bin/bash

# Get the directory where emcc is located
EMCC_DIR=$(dirname $(which emcc))
# Get the emsdk root (usually two directories up from emcc)
EMSDK_ROOT=$(cd "$EMCC_DIR/../.." && pwd)

echo "Emscripten compiler location: $(which emcc)"
echo "EMSDK root: $EMSDK_ROOT"

# Activate Emscripten environment directly
source "$EMSDK_ROOT/emsdk_env.sh" 2>/dev/null || {
  echo "Warning: Could not source emsdk_env.sh, assuming environment is already set up"
}

# Create build directory
mkdir -p build_wasm
cd build_wasm

# Configure with CMake, disabling tests
emcmake cmake .. -DBUILD_WASM=ON -DBUILD_TESTS=OFF

# Build
emmake make

# Copy output files to web directory
mkdir -p ../web
cp nes_debugger.js ../web/ 2>/dev/null || echo "Warning: nes_debugger.js not found"
cp nes_debugger.wasm ../web/ 2>/dev/null || echo "Warning: nes_debugger.wasm not found"

echo "Build complete. Files are in the web directory."
cd ..
