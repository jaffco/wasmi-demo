# wasmi-demo: WebAssembly interpreter running on a Daisy Seed

This project demonstrates a workflow for getting [WebAssembly](https://en.wikipedia.org/wiki/WebAssembly) applications running on the Daisy Seed via [Wasmi](https://github.com/wasmi-labs/wasmi), a small-footprint WebAssembly interpreter.

## Quick Start

1. First, [install the Daisy Toolchain](https://daisy.audio/tutorials/cpp-dev-env/#1-install-the-toolchain) && [Emscripten](https://emscripten.org).
2. Once installed, use `./init.sh` to configure your local copy of this repository.
3. This repository is configured for building SRAM apps. Connect your Daisy Seed via USB and [install a bootloader](https://flash.daisy.audio/) before proceeding.
4. With your device in program mode, use `./run.sh` (or `SHIFT+CMD+B` in VSCode) to build, flash, and run the program. Output will be logged to `log.txt`.

## Project Structure

```
wasmi-demo/
├── src/
│   └── main.cpp              # Main application with Wasmi integration
├── wasm-module/
│   ├── build/
│   │   ├── module.wasm       # Compiled WASM bytecode
│   │   └── module_wasm.h     # Embedded binary (auto-generated)
│   ├── module.cpp            # Module source code
│   └── build-wasm.sh         # Module build script (uses Emscripten)
├── wasmi-daisy/              # Wasmi C FFI wrapper (Rust crate)
├── libDaisy/                 # Daisy hardware library submodule
└── Makefile                  # Main build system
```

## Expected Output

Connect via USB serial to see:
- SDRAM allocator initialization
- Wasmi engine, store, and module loading
- Module instantiation and export resolution
- Process function test calls with various inputs
- Benchmark results with timing metrics
- Real-time performance analysis

## Wasmi Configuration

This build uses:
- **Interpreted execution** (Wasmi interpreter, no AOT compilation)
- **`no_std` Rust crate** compiled to a static library and linked into the C++ app
- **LTO + size optimization** (`opt-level = "z"`, `lto = "fat"`) for minimal flash footprint
- **Custom SDRAM allocator** — module data lives in the 64 MB external SDRAM
- **Cortex-M7 target** with FPU support

## Modifying the Module

Edit `wasm-module/module.cpp` and rebuild:

```bash
cd wasm-module
./build-wasm.sh
cd ..
make clean && make
```

## Troubleshooting

**"emcc not found"**
- Install Emscripten: `brew install emscripten`

**Build errors**
- Ensure submodules are initialized: `./init.sh`
- Check ARM toolchain is installed: `arm-none-eabi-gcc --version`

## Benchmarking

Checkout `FAUST` and `GEN` branches to compare performance across different WASM module code.