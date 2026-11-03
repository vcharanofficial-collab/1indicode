# Indicode Front End

This repository contains the C++20 front end for the proposed Indicode language. It currently provides source lexing, parsing into an AST, core name/type checks, and front-end inspection commands. LLVM, code generation, runtime execution, and AI/device backends are intentionally out of scope until the front end is complete and verified.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Commands

```sh
./build/indicode check examples/hello.indi
./build/indicode tokens examples/hello.indi
./build/indicode ast examples/hello.indi
./build/indicode typed examples/hello.indi
```

See [docs/PROGRESS.md](docs/PROGRESS.md) for scope, completion criteria, current progress, and the next milestone.