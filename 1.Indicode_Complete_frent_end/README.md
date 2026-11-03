# Indicode Front End

This project is building the C++20 front end for the proposed Indicode language, targeting a validated typed representation that can later be connected to an LLVM backend. **LLVM is not downloaded, vendored, or built here.** LLVM IR generation, runtime execution, and AI/device backends are not implemented in this front-end phase.

Current estimated completion: **95%** (weighted estimate 95.20%). The project has an initial lexer, an owning source manager with indexed line/column mapping, parser/AST and JSON serialization, core name/type checks, project-owned typed HIR lowering and verification, stable local symbol IDs, records/member access, integer ranges/iteration, nested `else if`, checked numeric conversions, static/symbolic tensor shapes, lists/indexing/iteration/named arguments, radix/separator integer literals, nested block comments, UTF-8 source validation, source-aware and JSON diagnostics with fix-its, an inspection CLI, and 16 CTest cases. The front end remains incomplete: modules/imports, complete semantics, and conformance testing are still missing. See [docs/FRONT_END_REMAINING_WORK.md](docs/FRONT_END_REMAINING_WORK.md) and its [PDF](Compiler_Front_End_Remaining_Work.pdf) for the front-end-only checklist. LLVM is strictly kept outside the project build. Read [docs/PROGRESS.md](docs/PROGRESS.md) for the evidence and percentage formula, [docs/ROADMAP_BEGINNER.md](docs/ROADMAP_BEGINNER.md) for the beginner guide, and [docs/COMMUNITY_RULES.md](docs/COMMUNITY_RULES.md) for contribution rules.

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
/tmp/indicode-build/indicode ast-json examples/records.indi
/tmp/indicode-build/indicode typed examples/hello.indi
/tmp/indicode-build/indicode typed-json examples/records.indi
/tmp/indicode-build/indicode check examples/records.indi
/tmp/indicode-build/indicode check examples/tensor_shapes.indi
/tmp/indicode-build/indicode --json-diagnostics check tests/fixtures/type_error.indi
```

`--json-diagnostics` prints machine-readable errors for editor/CI tools. It is an option before the command and source path.

`typed-json` prints verified project-owned HIR in a JSON envelope with `schemaVersion: 1`, including source spans and symbol IDs for a future backend adapter.

`ast-json` serializes the parsed AST before semantic analysis. Its version-1 JSON includes declarations, node kinds, source spans, expression metadata, and nested statements.

See [docs/PROGRESS.md](docs/PROGRESS.md) for measured progress, [docs/ROADMAP_BEGINNER.md](docs/ROADMAP_BEGINNER.md) for the plain-language roadmap and current syntax examples, and [docs/COMMUNITY_RULES.md](docs/COMMUNITY_RULES.md) for strict contribution rules.

## Production readiness

This is a buildable, tested front-end prototype for the documented language subset, not a production-ready general-purpose compiler. It does not generate LLVM IR or executables and still needs a complete language specification, conformance testing, fuzzing/sanitizer coverage, CI/release practices, and a stable compatibility policy. Do not rely on it for production workloads.