# wasmi-demo: WebAssembly interpreter running on a Daisy Seed

## What It Does

1. Inits logger and awaits serial connection
2. Initializes the wasmi interpreter with extensive logging
3. Loads a simple WebAssembly module that adds two numbers
4. Calls the module's `add` function with timing benchmark
5. Reports all results via serial logging

## Usage

1. First, [install the Daisy Toolchain](https://daisy.audio/tutorials/cpp-dev-env/#1-install-the-toolchain). 

2. Once installed, run the `init.sh` script to configure your local copy of this repository.

3. This repository is configured for building SRAM apps. Connect your Daisy Seed via USB and [install a bootloader](https://flash.daisy.audio/) before proceeding.

4. With your device in program mode, use `run.sh` (or `SHIFT+CMD+B` in VSCode) to build and flash to the Daisy.

5. To run `emcc` compiled Wasm, use `build-wasm.sh`. This will compile `add.cpp` and dump the Wasm into a buffer in `add_wasm.h`. <br> In `wasmiTest.cpp`, add a `#include` for `add_wasm.h`, and then replace the three instances of `wasm_add` with `add_wasm`. You can then use `run.sh` to build && flash. 

> [!NOTE]
> When developing for the Daisy, it is often useful to use serial monitoring for testing and debugging. If developing in VSCode, we recommend installing Microsoft's [serial monitor extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.vscode-serial-monitor), which will add easy access to serial monitoring via the terminal panel.