# Indicode Front End

This project is building the C++20 front end for the proposed Indicode language, targeting a validated typed representation that can later be connected to an LLVM backend. **LLVM is not downloaded, vendored, or built here.** LLVM IR generation, runtime execution, and AI/device backends are not implemented in this front-end phase.

Current estimated completion: **40%**. The project has an initial lexer, parser/AST, core name/type checks, project-owned typed HIR lowering and verification, inspection CLI, and focused tests. The HIR and language coverage are still a subset; complete semantics and diagnostics/tooling remain. LLVM is strictly kept outside the project build. Read [docs/PROGRESS.md](docs/PROGRESS.md) for the evidence and percentage formula, [docs/ROADMAP_BEGINNER.md](docs/ROADMAP_BEGINNER.md) for the beginner guide, and [docs/COMMUNITY_RULES.md](docs/COMMUNITY_RULES.md) for contribution rules.

## Build and test

```sh
cmake -S . -B /tmp/indicode-build
cmake --build /tmp/indicode-build
ctest --test-dir /tmp/indicode-build --output-on-failure
```

## Commands

```sh
/tmp/indicode-build/indicode check examples/hello.indi
/tmp/indicode-build/indicode tokens examples/hello.indi
/tmp/indicode-build/indicode ast examples/hello.indi
/tmp/indicode-build/indicode typed examples/hello.indi
```

See [docs/PROGRESS.md](docs/PROGRESS.md) for measured progress, [docs/ROADMAP_BEGINNER.md](docs/ROADMAP_BEGINNER.md) for the plain-language roadmap, and [docs/COMMUNITY_RULES.md](docs/COMMUNITY_RULES.md) for strict contribution rules.