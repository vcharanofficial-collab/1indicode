# Indicode Front End

This project is building the C++20 front end for the proposed Indicode language, targeting a validated typed representation that can later be connected to an LLVM backend. **LLVM is not downloaded, vendored, or built here.** LLVM IR generation, runtime execution, and AI/device backends are not implemented in this front-end phase.

Current estimated completion: **30%**. The project currently has an initial lexer, parser/AST, core name/type checks, inspection CLI, and tests. The standalone typed HIR, IR verifier, complete language semantics, and full diagnostics/tooling remain. See [docs/PROGRESS.md](docs/PROGRESS.md) for the calculation and detailed status.

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