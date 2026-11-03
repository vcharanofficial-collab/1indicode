# Indicode Front-End Progress

Updated: 2026-11-03

## Scope

This phase builds only the Indicode front end, targeting a validated typed representation that a future LLVM backend can consume. Its planned semantic surface includes the blueprint's tensor type and shape checks, which are not implemented yet. LLVM is not downloaded, vendored, or built in this repository. LLVM IR generation, machine-code generation, tensor execution, autodiff runtime/training, and device backends are outside the current phase.

## Completion

**Overall front-end completion: 30%**

The percentage measures progress toward the complete front-end boundary described in the blueprint, not progress toward LLVM itself or the full AI compiler. It is a weighted estimate: each area has a fixed weight, and its completion estimate is based on implemented, tested, documented acceptance behavior. The weighted result is 29.75%, rounded to 30%.

| Acceptance area | Weight | Area completion | Evidence / remaining work |
| --- | --- | --- |
| Project/build foundation | 5% | 80% | C++20, CMake, and CTest work; install/package/CI acceptance remains. |
| Source manager and diagnostics | 10% | 20% | Source file, spans, and records exist; owned source manager, UTF-8 validation, line mapping, rich rendering, and JSON output remain. |
| Lexer | 15% | 35% | Initial ASCII identifiers, selected keywords/operators, comments, newline, number/string tokens and errors; full Unicode, literals, escapes, and trivia remain. |
| Parser and AST | 15% | 35% | Functions, basic statements, blocks, calls, Pratt operators and a text printer; full grammar, recovery, AST snapshots, for-each, member/index, and named args remain. |
| Name resolution | 10% | 30% | Local scopes and basic variable/function lookup; declaration phases, globals/modules/imports/visibility and robust symbol identities remain. |
| Type checking and semantic validation | 15% | 30% | Core scalar inference, same-type operations, mutability, signatures, returns and integer ranges; full conversions, control-flow/effect/ownership and tensor shape rules remain. |
| Typed HIR and verification | 15% | 0% | Current types annotate AST nodes; separate typed HIR, lowering, and IR verifier are not implemented. |
| CLI and diagnostics tooling | 5% | 35% | `check`, `tokens`, `ast`, `typed` plus plain diagnostics; JSON diagnostics, editor interfaces, and stable source snippets/fix-its remain. |
| Tests, docs, and integration | 10% | 40% | Five CTest cases and a sample; broad conformance, fuzz/property tests, and CI remain. |

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
- The last verified CLI executable measured 352,136 bytes before build artifacts were cleaned; this is not a size guarantee for future builds or a runtime-memory limit.

## Current limitations

- Identifiers are ASCII-only. UTF-8 bytes are retained in strings, but Unicode validation and Unicode identifier rules are not implemented.
- Numeric bases, digit separators, suffixes, complete escape validation, and block comments are not implemented.
- Semantic checking is a core subset; numeric widening/conversions, full return-flow analysis, overloads, generics, module resolution, and complete diagnostics rendering remain.
- Parser coverage is still a subset: for-each loops, member/index expressions, named arguments, complete newline rules, AST printer snapshot tests, and full grammar recovery remain.
- Numeric conversion is exact-type-only outside contextual integer literals; floating-point range/precision checks and the complete conversion policy remain.
- A separate typed HIR and verifier are required before the front end can be called ready for a future LLVM adapter; current inferred types are attached to AST nodes only.

## Milestones

1. **Source model and lexer:** complete UTF-8/source management, token coverage, literal validation, and lexer tests.
2. **Parser and AST:** implement the documented core grammar, precedence, source spans, AST printer, and recovery tests.
3. **Names and types:** add scopes, symbol resolution, core types, inference, conversion/range checks, and diagnostics.
4. **Core semantic validation:** add function/control-flow checks, mutability rules, and negative tests.
5. **Typed HIR:** lower valid source to typed HIR and verify invariants.
6. **Front-end CLI and release checks:** provide `check`, `tokens`, `ast`, and `typed`; add JSON diagnostics, integration tests, and usage docs.

LLVM/backend work begins only after these front-end milestones are complete and accepted. The front end is being designed to connect to LLVM later without requiring the LLVM framework to be downloaded into this repository now.

## Latest verification

- A fresh out-of-tree CMake configure/build succeeded with GCC 13.3.
- All 5 CTest cases passed: lexer, parser, semantic analysis, valid CLI check, and expected rejection of a type error; the documented example also passes `indicode check`.
- The temporary verification build and the project's generated `build/` directory were removed after validation. No compiled artifacts remain in the project folder.