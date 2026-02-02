# wasmi-demo: WebAssembly interpreter running on a Daisy Seed

This project demonstrates a workflow for getting [WebAssembly](https://en.wikipedia.org/wiki/WebAssembly) applications running on the Daisy Seed via [Wasmi](https://github.com/wasmi-labs/wasmi), a small-footprint WebAssembly interpreter.

## Quick Start

1. First, [install the Daisy Toolchain](https://daisy.audio/tutorials/cpp-dev-env/#1-install-the-toolchain). 
2. Once installed, use `./init.sh` to configure your local copy of this repository. 
3. This repository is configured for building SRAM apps. Connect your Daisy Seed via USB and [install a bootloader](https://flash.daisy.audio/) before proceeding.
4. With your device in program mode, use `./run.sh` (or `SHIFT+CMD+B` in VSCode) to build, flash, and run the program. Output will be logged to `log.txt`.