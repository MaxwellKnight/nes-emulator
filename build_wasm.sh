#!/bin/bash
set -e  # Exit on error

# Print script banner
echo "======================================"
echo "NES 6502 CPU Debugger - WASM Build"
echo "======================================"

# Get the directory where emcc is located
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten compiler (emcc) not found in PATH"
    echo "Please make sure Emscripten is installed and activated"
    exit 1
fi

EMCC_DIR=$(dirname $(which emcc))
# Get the emsdk root (usually two directories up from emcc)
mMSDK_ROOT=$(cd "$EMCC_DIR/../.." && pwd)
echo "Emscripten compiler location: $(which emcc)"
echo "EMSDK root: $EMSDK_ROOT"

# Activate Emscripten environment
source "$EMSDK_ROOT/emsdk_env.sh" 2>/dev/null || {
  echo "Warning: Could not source emsdk_env.sh, assuming environment is already set up"
}

# Create build directory
mkdir -p build_wasm
cd build_wasm

# Configure with CMake
echo "Configuring project with CMake..."
emcmake cmake .. -DBUILD_WASM=ON -DBUILD_TESTS=OFF

# Build
echo "Building project..."
emmake make -j$(nproc)

# Check if output files exist
if [ ! -f "nes_debugger.js" ] || [ ! -f "nes_debugger.wasm" ]; then
    echo "Error: Build files not found!"
    echo "Debug information:"
    ls -la
    exit 1
fi

# Create web directory if it doesn't exist
mkdir -p ../web

# Copy output files to web directory
echo "Copying output files to web directory..."
cp nes_debugger.js ../web/
cp nes_debugger.wasm ../web/

# Verify files were copied
if [ -f "../web/nes_debugger.js" ] && [ -f "../web/nes_debugger.wasm" ]; then
    echo "✅ Build complete! Files copied to web directory."
else
    echo "❌ Error: Failed to copy output files to web directory."
    exit 1
fi

# Create .htaccess file to set correct MIME type for WASM files
echo "Creating .htaccess file for proper MIME types..."
cat > ../web/.htaccess << EOL
# Set the correct MIME type for WebAssembly files
<Files *.wasm>
    AddType application/wasm .wasm
</Files>
EOL

echo ""
echo "======================================"
echo "To test your application:"
echo "1. Navigate to the web directory: cd ../web"
echo "2. Start a local server: python -m http.server 8080"
echo "3. Open your browser at: http://localhost:8080"
echo "======================================"

cd ..
