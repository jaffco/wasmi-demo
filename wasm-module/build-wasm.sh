#!/bin/bash
set -e

echo "Building WASM module for wasmi..."

# Clean old build artifacts
rm -f build/module.wasm build/module_wasm.h

# Create build directory
mkdir -p build

# Check for emcc
if ! command -v emcc &> /dev/null; then
    echo "ERROR: emscripten not found!"
    echo "Please install with: brew install emscripten"
    exit 1
fi

echo "Using emscripten: $(which emcc)"

# Compile C++ to WASM using emscripten
echo "Compiling C++ to WASM..."
emcc \
    -O2 \
    -sSTANDALONE_WASM \
    -sEXPORTED_RUNTIME_METHODS=[] \
    -sEXPORTED_FUNCTIONS=_process \
    -sERROR_ON_UNDEFINED_SYMBOLS=0 \
    --no-entry \
    -DGENLIB_NO_JSON \
    -DGENLIB_USE_FLOAT32 \
    -DGENLIB_NO_DENORM_TEST \
    -DWIN32 \
    -IPhhhsrrr/gen_dsp \
    -o build/module.wasm \
    module.cpp \
    Phhhsrrr/gen_dsp/genlib.cpp

echo "WASM module size: $(wc -c < build/module.wasm) bytes"

# Convert WASM binary to C header array
echo "Embedding WASM in C header..."
xxd -i -n module.wasm build/module.wasm > build/module_wasm.h

echo ""
echo "================================"
echo "Module build complete!"
echo "================================"
echo "Generated files:"
echo "  - build/module.wasm ($(wc -c < build/module.wasm) bytes)"
echo "  - build/module_wasm.h (embedded)"
echo ""
