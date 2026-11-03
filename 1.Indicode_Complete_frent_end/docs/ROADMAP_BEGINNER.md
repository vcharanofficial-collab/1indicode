# Indicode Front-End Roadmap for Beginners

Updated: 2026-11-03

## What Are We Building?

We are building the part of a compiler that reads Indicode source code and checks what it means. It should eventually turn a correct program into a typed, verified Indicode representation that a separately installed LLVM backend can consume.

LLVM is a large, separate compiler framework. This project does **not** download, copy, or build LLVM. Our front end and its tests must keep working without LLVM. Connecting an optional backend is a later project stage, after the front end has a stable typed representation.

The PDF is a blueprint, not a list of features already completed. Its 777 concepts are a long-term catalogue, not 777 built-in types or a promise to implement every idea immediately.

## Current Overall Progress: 40%

This percentage means estimated completion of the **front-end work needed before a future LLVM adapter**. It does not mean 40% of LLVM, machine-code generation, a GPU compiler, or the AI runtime is complete.

The estimate uses the weighted acceptance areas in `PROGRESS.md`:

- Before this update: 29.75%.
- Typed HIR has a 15% weight. Its current completion is estimated at 65%, based on project-owned typed nodes, AST lowering, a verifier, a printer, tests, and CLI use. Contribution: 15% x 65% = 9.75%.
- HIR tests plus the beginner roadmap and community rules improve the tests/docs area from 40% to 45%, contributing another 0.5%.
- New total: 29.75% + 9.75% + 0.5% = **40.0%**.

This is an estimate, not a marketing score. It may go down if a review finds missing acceptance behavior. Passing tests prove only the tested cases, not completeness of the language.

## What Already Works?

- C++20/CMake builds the front-end without LLVM.
- The lexer splits a small subset of Indicode text into tokens.
- The parser builds an AST (Abstract Syntax Tree: a tree-shaped record of the code's structure).
- Semantic checks catch selected name, type, mutability, function-call, return, and integer-range errors.
- The compiler lowers checked AST into project-owned typed HIR (High-level Intermediate Representation: a simpler, explicitly typed form of the program).
- The HIR verifier checks required type annotations, expression/operator types, function calls, and return types.
- `indicode check`, `tokens`, `ast`, and `typed` work for the current subset.
- Six CTest cases cover lexer, parser, semantics, HIR, and CLI acceptance/rejection.

## Roadmap

| Stage | Status | In plain language | Completion evidence |
| --- | --- | --- | --- |
| 1. Project foundation | Mostly done | Build the project without large external frameworks. | C++20/CMake build and CTest work without LLVM or network downloads. |
| 2. Source and lexer | Started | Read characters and recognize names, numbers, strings, punctuation, and errors. | UTF-8 validation, full literal/comment coverage, exact source positions, negative tests. |
| 3. Parser and AST | Started | Recognize the language's grammar and record its structure. | Full core grammar, recovery, precedence, spans, AST golden tests. |
| 4. Names and types | Started | Decide what names refer to and whether operations use compatible types. | Scope rules, conversions, overflow, return paths, and negative diagnostics are specified and tested. |
| 5. Typed HIR | **Started; core slice implemented** | Convert checked source into our own typed representation. | More language coverage, stable IDs, effects/conversions, verifier invariants and serialization tests. |
| 6. Diagnostics and tools | Started | Make errors useful to people and editors. | Stable source excerpts, JSON output, precise spans, and recovery tests. |
| 7. Front-end acceptance | Not complete | Prove the documented front-end subset works reliably. | Conformance suite, fuzz/property tests, sanitizers, CI, docs, and repeatable clean builds. |
| 8. Optional LLVM adapter | Not started | Connect verified Indicode HIR to an external LLVM installation. | Separate opt-in adapter, no vendored LLVM, backend integration tests, documented external setup. |

## What Must Happen Before LLVM Is Connected?

1. Finish and version the front-end grammar and type/conversion rules.
2. Expand lexer/parser/resolver/type-checker tests to cover valid and invalid examples from the PDF's v1 subset.
3. Make the typed HIR stable, independently verifiable, and documented.
4. Keep source spans and diagnostics usable through lowering.
5. Pass clean offline builds, the complete test suite, fuzz/sanitizer checks, and beginner examples.
6. Agree on a separate LLVM adapter design. Only that optional adapter may locate an externally installed LLVM; ordinary front-end builds must remain independent.

## How to Build and Try It

From this project directory, an out-of-tree build keeps generated files away from source:

```sh
cmake -S . -B /tmp/indicode-build
cmake --build /tmp/indicode-build
ctest --test-dir /tmp/indicode-build --output-on-failure
/tmp/indicode-build/indicode check examples/hello.indi
/tmp/indicode-build/indicode typed examples/hello.indi
```

A successful check means the source passed the current subset's front-end checks. It does not mean the program was executed or compiled to machine code.

## Glossary

- **Lexer:** turns characters into tokens such as names, numbers, and symbols.
- **Parser:** checks token order against the language grammar.
- **AST:** a tree describing the source code's structure.
- **Name resolution:** connects each use of a name to its declaration.
- **Type checking:** checks that values and operations are compatible.
- **HIR:** this project's typed, higher-level representation between source code and a future backend.
- **Verifier:** checks that an HIR program obeys the representation's rules before another compiler stage uses it.
- **LLVM adapter:** a future, separate component that translates verified Indicode representation for an externally provided LLVM framework.
