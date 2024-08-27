# MCTP Over USB / Performance Assessment (Draft).

This repository contains code, documentation, scripts, and other resources to assist in accurately assessing the Xtensa LX7 cycle count for the implementation of MCTP (Management Component Transport Protocol) over USB.

## Prerequisites

Before building and running this project, ensure that you have the following tools and environment set up:

1. **Xtensa SDK for the LX7**
    - Required tools:
        - `xt-clang`, `xt-as`, `xt-ld`: Compiler, assembler, and linker.
        - `xt-gdb`: Debugger.
        - `xt-run`: Xtensa instruction set simulator.

2. **Operating System**
    - The project was developed on Fedora 34. It should also work on other Linux distributions, preferably more modern versions.

3. **Python**
    - Python is required if you need to convert Xtensa exported assembly files to instruction counts.

## Dependencies

This project uses modified versions of the following libraries:

1. **libmctp** - Part of OpenBMC: [openbmc/libmctp](https://github.com/openbmc/libmctp)
2. **libcargs** - A more sensible method of parsing input arguments: [likle/cargs](https://github.com/likle/cargs)
3. **Linked Lists (uthash)** - Essential linked list functionality: [uthash](https://troydhanson.github.io/uthash/)

## Code Organization

- **`src/`** - Contains the USB/MCTP application code.
- **`libmctp/`** - A modified version of the libmctp library, where most of the `malloc()` and `free()` calls have been replaced with a simplified free/list queue.
- **`resources/`** - Miscellaneous files and resources.

## Building and Testing

To build and test the project, ensure the Xtensa SDK is installed and its environment variables are correctly exported.

### Available Commands:

- **Build**
    ```bash
    make release
    make debug
    ```

- **Run**
    ```bash
    make run
    ```
    This command builds an optimized version of the project and executes it using `xt-run`.
