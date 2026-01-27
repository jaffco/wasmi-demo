# WASM Module Build Instructions

This directory contains the module that will be compiled to WASM for interpretive execution with wasmi on the Daisy Seed.

## Prerequisites

1. **Emscripten** - Required for compiling C++ to WebAssembly
   - Install with: `brew install emscripten`

## Building

```bash
./build-wasm.sh
```

This will:
1. Compile `module.cpp` to `build/module.wasm`
2. Generate `build/module_wasm.h` with embedded binary data for use in the main application

## Manual Build Steps

If you need to build manually:

```bash
# Compile C++ to WASM
emcc \
    -O2 \
    -sSTANDALONE_WASM \
    -sEXPORTED_RUNTIME_METHODS=[] \
    -sEXPORTED_FUNCTIONS=_add \
    -sERROR_ON_UNDEFINED_SYMBOLS=0 \
    --no-entry \
    -o build/module.wasm \
    module.cpp

# Embed in header
xxd -i build/module.wasm > build/module_wasm.h
```
