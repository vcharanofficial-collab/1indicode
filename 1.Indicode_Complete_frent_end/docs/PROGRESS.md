# Indicode Front-End Progress

Updated: 2026-11-03

## Scope

This phase builds only the Indicode front end, targeting a validated typed representation that a future LLVM backend can consume. Its planned semantic surface includes the blueprint's tensor type and shape checks, which are not implemented yet. LLVM is not downloaded, vendored, or built in this repository. LLVM IR generation, machine-code generation, tensor execution, autodiff runtime/training, and device backends are outside the current phase.

## Completion

**Overall front-end completion: 40%**

The percentage measures progress toward the complete front-end boundary described in the blueprint, not progress toward LLVM itself or the full AI compiler. It is a weighted estimate: each area's weight is fixed, and completion credit requires implemented, tested, documented acceptance behavior. Before this update, the weighted result was 29.75%. Typed HIR has a 15% weight and is estimated at 65% complete based on initial lowering, verification, printing, CLI use, and tests; that adds 9.75 points. New HIR coverage and beginner/community documentation raise the tests/docs area from 40% to 45%, adding 0.5 points. Current total: 29.75% + 9.75% + 0.5% = 40.0%.

| Acceptance area | Weight | Area completion | Evidence / remaining work |
| --- | --- | --- |
| Project/build foundation | 5% | 80% | C++20, CMake, and CTest work; install/package/CI acceptance remains. |
| Source manager and diagnostics | 10% | 20% | Source file, spans, and records exist; owned source manager, UTF-8 validation, line mapping, rich rendering, and JSON output remain. |
| Lexer | 15% | 35% | Initial ASCII identifiers, selected keywords/operators, comments, newline, number/string tokens and errors; full Unicode, literals, escapes, and trivia remain. |
| Parser and AST | 15% | 35% | Functions, basic statements, blocks, calls, Pratt operators and a text printer; full grammar, recovery, AST snapshots, for-each, member/index, and named args remain. |
| Name resolution | 10% | 30% | Local scopes and basic variable/function lookup; declaration phases, globals/modules/imports/visibility and robust symbol identities remain. |
| Type checking and semantic validation | 15% | 30% | Core scalar inference, same-type operations, mutability, signatures, returns and integer ranges; full conversions, control-flow/effect/ownership and tensor shape rules remain. |
| Typed HIR and verification | 15% | 65% | Project-owned typed HIR, AST lowering, type/operator/call/return checks, printer, CLI integration, and tests exist; stable symbol IDs, full semantics, serialization, and broader verifier coverage remain. |
| CLI and diagnostics tooling | 5% | 35% | `check`, `tokens`, `ast`, `typed` plus plain diagnostics; JSON diagnostics, editor interfaces, and stable source snippets/fix-its remain. |
| Tests, docs, and integration | 10% | 45% | Six CTest cases, the sample, strict community rules, and a beginner roadmap; broad conformance, fuzz/property tests, and CI remain. |

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
- Added project-owned typed HIR types and AST-to-HIR lowering without LLVM headers, libraries, downloads, or build integration.
- Added an HIR verifier for type annotations, expression structure, operator consistency, call signatures, statement types, and return types; `indicode typed` now prints verified HIR.
- Added HIR tests for valid lowering/verification, typed output, and rejection of an invalid type annotation.
- Added strict community contribution rules and a beginner-readable roadmap explaining the LLVM boundary and percentage calculation.
- The last verified CLI executable measured 352,136 bytes before build artifacts were cleaned; this is not a size guarantee for future builds or a runtime-memory limit.

## Current limitations

- Identifiers are ASCII-only. UTF-8 bytes are retained in strings, but Unicode validation and Unicode identifier rules are not implemented.
- Numeric bases, digit separators, suffixes, complete escape validation, and block comments are not implemented.
- Semantic checking is a core subset; numeric widening/conversions, full return-flow analysis, overloads, generics, module resolution, and complete diagnostics rendering remain.
- Parser coverage is still a subset: for-each loops, member/index expressions, named arguments, complete newline rules, AST printer snapshot tests, and full grammar recovery remain.
- Numeric conversion is exact-type-only outside contextual integer literals; floating-point range/precision checks and the complete conversion policy remain.
- Typed HIR/verifier currently cover the existing language subset only; they need complete semantic coverage, stable symbol identities, stronger validation, serialization/versioning, and conformance tests before an LLVM adapter is ready.

## Milestones

1. **Source model and lexer:** complete UTF-8/source management, token coverage, literal validation, and lexer tests.
2. **Parser and AST:** implement the documented core grammar, precedence, source spans, AST printer, and recovery tests.
3. **Names and types:** add scopes, symbol resolution, core types, inference, conversion/range checks, and diagnostics.
4. **Core semantic validation:** add function/control-flow checks, mutability rules, and negative tests.
5. **Typed HIR:** initial lowering, printer, verifier, and tests are implemented; complete semantics, versioning, serialization, and robust invariants remain.
6. **Front-end CLI and release checks:** `check`, `tokens`, `ast`, and `typed` exist; add JSON diagnostics, full integration/conformance tests, fuzzing, and usage docs.

LLVM/backend work begins only after these front-end milestones are complete and accepted. The front end is being designed to connect to LLVM later without requiring the LLVM framework to be downloaded into this repository now.

## Latest verification

- A fresh out-of-tree CMake configure/build succeeded with GCC 13.3.
- All 6 CTest cases passed: lexer, parser, semantic analysis, HIR lowering/verifier, valid CLI check, and expected rejection of a type error; the documented example also passes `indicode check`.
- The temporary verification build and the project's generated `build/` directory were removed after validation. No compiled artifacts remain in the project folder.