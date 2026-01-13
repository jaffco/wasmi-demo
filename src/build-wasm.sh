#!/bin/bash

# Create a temporary wrapper to force C linkage without modifying source
cat > /tmp/add_wrapper.cpp << 'EOF'
extern "C" {
#include "add.cpp"
}
EOF

# Compile C++ to WebAssembly with exported functions
# The wrapper provides extern "C" linkage without modifying the original source
emcc /tmp/add_wrapper.cpp -O2 -o add.wasm \
  -I. \
  -sSTANDALONE_WASM \
  -sEXPORTED_RUNTIME_METHODS=[] \
  -sEXPORTED_FUNCTIONS=_add \
  --no-entry

# Convert WASM binary to C header array
xxd -i add.wasm > add_wasm.h

# Cleanup
rm /tmp/add_wrapper.cpp
rm add.wasm

echo "✓ Built a.out.wasm and generated add_wasm.h"
echo "✓ Function 'add' exported with C linkage"