# Indicode Front-End Progress

Updated: 2026-11-03

## Scope

This project implements the Indicode compiler front end only. The front end ends at a validated, typed representation and diagnostics. LLVM integration, machine-code generation, runtime, tensors, autodiff, and device backends are not part of this phase.

## Completion

**Overall front-end completion: 33%**

The percentage is a coarse engineering estimate across the complete front-end acceptance areas below, not a feature-count claim. A feature is complete only when implemented, tested, and documented.

| Area | Status | Notes |
| --- | --- | --- |
| Project/build foundation | In progress | C++20 and CMake are configured; CI and install packaging remain. |
| Source model and diagnostics | Started | Source file, byte offsets, line/column positions, and diagnostic records exist; source manager/rendering remain. |
| Lexer | Started | ASCII identifiers, core keywords/operators, comments, newline separators, numeric/string tokens, and recoverable errors. |
| Parser and AST | Started | Function declarations, basic statements/blocks, calls, unary/binary expressions, Pratt precedence, and parser recovery diagnostics. |
| Name resolution | Started | Lexical scopes, variable/function lookup, duplicate-name checks, and unresolved-name diagnostics. |
| Type checking and semantic validation | Started | Core scalar type inference, exact-match assignments/calls/returns, mutability, conditions, and checked destination-type integer literal ranges. |
| Typed HIR and verification | Started | AST nodes carry inferred expression types and resolved symbol names; a separate verified HIR remains. |
| CLI, JSON diagnostics, and integration tests | Started | `check`, `tokens`, `ast`, and `typed` are implemented; JSON diagnostics and broader integration remain. |

## Work completed

- Added a C++20 CMake library and a CTest-based lexer test executable.
- Added source locations/spans, source-file storage, diagnostic records, and token definitions.
- Implemented initial lexing for identifiers, selected keywords, numeric and string literals, punctuation, operators, `#` line comments, and newline separators.
- Added stable lexer error codes for unsupported characters, malformed logical operators, malformed exponents, and unterminated strings.
- Added tests covering function syntax, CRLF line endings, comments, strings, EOF, and recoverable malformed literals.
- Configured and built with GCC 13.3 and CMake; the lexer CTest target passes (1/1).
- Added an AST model and parser for function signatures, variable declarations, assignment, return, if/else, while, repeat, calls, and precedence-based expressions.
- Added parser tests for function/body structure, expression precedence, recoverable syntax errors, and empty input; lexer and parser CTest targets pass (2/2).
- Added scope-based name lookup, function signature checks, inferred types on expressions, basic mutability/type checks, and return-path validation for simple branches.
- Added the `indicode` CLI with `check`, `tokens`, `ast`, and `typed` commands, plus valid/invalid CLI integration fixtures.
- Added a source-oriented AST/typed-tree printer and documented local build and command usage.
- Added context-typed integer literals with range checks for Byte/U8, Int2/UInt2, Int/UInt, and Long/ULong; signed minimum values are checked without overflow.
- Added `examples/hello.indi` as the first runnable end-to-end front-end sample.

## Current limitations

- Identifiers are ASCII-only. UTF-8 bytes are retained in strings, but Unicode validation and Unicode identifier rules are not implemented.
- Numeric bases, digit separators, suffixes, complete escape validation, and block comments are not implemented.
- Semantic checking is a core subset; numeric widening/conversions, full return-flow analysis, overloads, generics, module resolution, and complete diagnostics rendering remain.
- Parser coverage is still a subset: for-each loops, member/index expressions, named arguments, complete newline rules, AST printing, and full grammar recovery remain.
- Numeric conversion is exact-type-only outside contextual integer literals; floating-point range/precision checks and the complete conversion policy remain.

## Milestones

1. **Source model and lexer:** complete UTF-8/source management, token coverage, literal validation, and lexer tests.
2. **Parser and AST:** implement the documented core grammar, precedence, source spans, AST printer, and recovery tests.
3. **Names and types:** add scopes, symbol resolution, core types, inference, conversion/range checks, and diagnostics.
4. **Core semantic validation:** add function/control-flow checks, mutability rules, and negative tests.
5. **Typed HIR:** lower valid source to typed HIR and verify invariants.
6. **Front-end CLI and release checks:** provide `check`, `tokens`, `ast`, and `typed`; add JSON diagnostics, integration tests, and usage docs.

LLVM/backend work begins only after these front-end milestones are complete and accepted.

## Latest verification

- `cmake -S . -B build` and `cmake --build build` succeed with GCC 13.3.
- `ctest --test-dir build --output-on-failure` passes all 5 tests: lexer, parser, semantic analysis, valid CLI check, and expected rejection of a type error.