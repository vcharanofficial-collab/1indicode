# Indicode Front-End Progress

Updated: 2026-11-03

## Scope

This phase builds only the Indicode front end, targeting a validated typed representation that a future LLVM backend can consume. Its planned semantic surface includes the blueprint's tensor type and shape checks, which are not implemented yet. LLVM is not downloaded, vendored, or built in this repository. LLVM IR generation, machine-code generation, tensor execution, autodiff runtime/training, and device backends are outside the current phase.

## Completion

**Overall front-end completion: 80%**

The percentage measures progress toward the complete front-end boundary described in the blueprint, not progress toward LLVM itself or the full AI compiler. It is a weighted estimate: each area's weight is fixed, and completion credit requires implemented, tested, documented acceptance behavior. The preceding estimate was 69.0%. Record declarations/construction/member typing, checked integer conversions, and their HIR validation were added and tested; the revised weighted total is **80.05%, rounded to 80%**.

| Acceptance area | Weight | Area completion | Evidence / remaining work |
| --- | --- | --- |
| Project/build foundation | 5% | 80% | C++20, CMake, and CTest work; install/package/CI acceptance remains. |
| Source manager and diagnostics | 10% | 55% | Source spans, source-line/caret rendering, JSON escaping and structured spans; owned source manager, display-width mapping, fix-its and richer JSON schema remain. |
| Lexer | 15% | 89% | ASCII identifiers, UTF-8 strings, escapes, binary/octal/hex integers, digit separators, and nested block comments with newline preservation; Unicode identifiers, suffix policy, trivia preservation, and complete numeric grammar remain. |
| Parser and AST | 15% | 76% | Functions, blocks, calls, precedence, typed lists, indexing, for-each, named arguments, record declarations, and member access; ranges, full recovery and AST snapshots remain. |
| Name resolution | 10% | 70% | Lexical scopes, function/variable/parameter lookup, record type names and field lookup, and scoped for-each bindings; modules/imports and stable symbol identities remain. |
| Type checking and semantic validation | 15% | 82% | Scalar/radix integer inference, literal range checks, checked integer conversions, named parameter binding, homogeneous lists, record construction/field types, indexing, loops and mutability; floating conversions, ownership/effects and tensor shapes remain. |
| Typed HIR and verification | 15% | 94% | Project-owned typed HIR covers records, member access, checked integer conversions, collections and loops; verifier checks record/conversion/collection/index/loop/call contracts. Stable IDs, serialization/versioning and broader invariants remain. |
| CLI and diagnostics tooling | 5% | 80% | `check`, `tokens`, `ast`, `typed`, source excerpts/carets and opt-in JSON diagnostics; editor interfaces, fix-its and diagnostics schema stability remain. |
| Tests, docs, and integration | 10% | 84% | Nine CTest cases, including end-to-end record and checked-conversion checks, plus radix/named-argument/comment/Unicode/collection coverage; conformance, fuzz/property tests, sanitizers and CI remain. |

Updated weighted estimate: 4.0 + 5.5 + 13.35 + 11.4 + 7.0 + 12.3 + 14.1 + 4.0 + 8.4 = **80.05%, rounded to 80%**. This remains an estimate for the front-end boundary, not a claim that every PDF feature is implemented.

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
- Added source-line/caret text diagnostics and opt-in `--json-diagnostics` with stable codes, escaped messages, file paths, and structured source spans.
- Added homogeneous list literals, list/text indexing, and `for item in items` for-each loops through AST, semantic checking, HIR, verifier, printer, and CLI.
- Added UTF-8 validation for string contents, common string escape checks, `\u{...}` Unicode scalar validation, and Char/Text inference by scalar count.
- Added lexer, semantic, HIR, diagnostic, and JSON CLI regression tests for the new behavior.
- Documented supported list/iteration/index and Unicode-string rules, including positive/negative examples and current limits.
- Added strict community contribution rules and a beginner-readable roadmap explaining the LLVM boundary and percentage calculation.
- Added binary/octal/hexadecimal integer literals and underscore separators with malformed-digit/separator diagnostics and destination range checks.
- Added named function arguments end-to-end, including parameter-name resolution, duplicate/unknown/missing argument errors, and HIR preservation.
- Added nested block comments, preserved their newline separators, and diagnosed unterminated comments.
- Added record declarations, named record construction, and typed member access through parsing, semantic analysis, HIR lowering/verification, printers, and CLI coverage.
- Added checked integer conversion calls to all supported integer destination types; conversion requires a numeric input and `checked: true`, and integer constants are range-checked before lowering.

## Current limitations

- Identifiers are ASCII-only. UTF-8 is validated in string contents, but Unicode identifier rules and full Unicode display-column mapping are not implemented.
- Numeric suffixes, raw strings, and the complete string escape repertoire remain incomplete.
- Semantic checking is a core subset; only checked conversions to integer destinations are implemented. Numeric widening, floating conversions, full return-flow analysis, overloads, generics, module resolution, and complete diagnostics rendering remain.
- Parser coverage is still a subset: ranges, complete newline rules, AST printer snapshot tests, and full grammar recovery remain.
- Typed HIR/verifier currently cover the existing language subset only; they need complete semantic coverage, stable symbol identities, stronger validation, serialization/versioning, and conformance tests before an LLVM adapter is ready.

## Milestones

1. **Source model and lexer:** complete UTF-8/source management, token coverage, literal validation, and lexer tests.
2. **Parser and AST:** implement the documented core grammar, precedence, source spans, AST printer, and recovery tests.
3. **Names and types:** add scopes, symbol resolution, core types, inference, conversion/range checks, and diagnostics.
4. **Core semantic validation:** add function/control-flow checks, mutability rules, and negative tests.
5. **Typed HIR:** initial lowering, printer, verifier, and tests are implemented; complete semantics, versioning, serialization, and robust invariants remain.
6. **Front-end CLI and release checks:** `check`, `tokens`, `ast`, `typed`, and initial JSON diagnostics exist; complete the JSON schema and add full integration/conformance tests, fuzzing, and usage docs.

LLVM/backend work begins only after these front-end milestones are complete and accepted. The front end is being designed to connect to LLVM later without requiring the LLVM framework to be downloaded into this repository now.

## Latest verification

- A fresh out-of-tree CMake configure/build succeeded with GCC 13.3.
- All 9 CTest cases passed, including lexer, parser, semantic analysis, HIR lowering/verifier, diagnostic formatting, valid CLI checks for the standard and record examples, expected rejection of a type error, and JSON diagnostic CLI validation.
- The verification build was kept outside the project at `/tmp/indicode-progress-build`; no generated build files were added to the repository.