# Indicode Front-End Progress

Updated: 2026-11-03

## Scope

This phase builds only the Indicode front end, targeting a validated typed representation that a future LLVM backend can consume. It now includes an initial tensor type and static/symbolic shape-compatibility subset; tensor operations, broadcasting, dynamic shape guards, and device checks are not implemented. LLVM is not downloaded, vendored, or built in this repository. LLVM IR generation, machine-code generation, tensor execution, autodiff runtime/training, and device backends are outside the current phase.

## Completion

**Overall front-end completion: 91%**

The percentage measures progress toward the complete front-end boundary described in the blueprint, not progress toward LLVM itself or the full AI compiler. It is a weighted estimate: each area's weight is fixed, and completion credit requires implemented, tested, documented acceptance behavior. The update adds nested `else if` parsing and analysis, HIR return-path verification, and diagnostic caret alignment for tabs and UTF-8 scalar columns. The updated weighted total is **90.65%, rounded to 91%**.

| Acceptance area | Weight | Area completion | Evidence / remaining work |
| --- | --- | --- |
| Project/build foundation | 5% | 80% | C++20, CMake, and CTest work; install/package/CI acceptance remains. |
| Source manager and diagnostics | 10% | 80% | Source spans, source-line/caret rendering aligned across tabs and UTF-8 scalar columns, JSON escaping and structured spans; owned source manager, terminal display-width handling, fix-its and richer JSON schema remain. |
| Lexer | 15% | 89% | ASCII identifiers, UTF-8 strings, escapes, binary/octal/hex integers, digit separators, and nested block comments with newline preservation; Unicode identifiers, suffix policy, trivia preservation, and complete numeric grammar remain. |
| Parser and AST | 15% | 94% | Functions, blocks, calls, precedence, typed lists, indexing, for-each, named arguments, records/member access, tensor element/shape types, integer ranges, and nested `else if`; full grammar recovery and AST snapshots remain. |
| Name resolution | 10% | 75% | Lexical scopes, function/variable/parameter lookup, record type names/fields, and scoped for-each bindings; modules/imports and stable symbol identities remain. |
| Type checking and semantic validation | 15% | 97% | Scalar/radix integer inference, literal range checks, checked numeric conversions, record construction/field types, tensor element/shape compatibility, integer range bounds, indexing, nested conditional return analysis, loops and mutability; tensor operations, ownership/effects and full conversion policies remain. |
| Typed HIR and verification | 15% | 99% | Project-owned typed HIR covers records, member access, tensor types, checked numeric conversions, collections, and integer ranges; verifier checks return-path completeness plus range bound/result consistency and record/conversion/collection/index/loop/call contracts. Stable IDs, serialization/versioning and broader invariants remain. |
| CLI and diagnostics tooling | 5% | 90% | `check`, `tokens`, `ast`, `typed`, source excerpts/carets aligned across tabs and UTF-8 scalar columns, JSON diagnostics, and tensor/record example checks; terminal-specific display widths, editor interfaces, fix-its and diagnostics schema stability remain. |
| Tests, docs, and integration | 10% | 98% | Eleven CTest cases cover parser recovery, nested `else if` return paths, HIR return verification, deterministic malformed-input smoke testing, tensor shapes, records, numeric conversions, integer ranges, diagnostics, and CLI; full conformance, coverage-guided fuzzing, sanitizers and CI remain. |

Updated weighted estimate: 4.0 + 8.0 + 13.35 + 14.1 + 7.5 + 14.55 + 14.85 + 4.5 + 9.8 = **90.65%, rounded to 91%**. This is a rounded estimate for the front-end boundary, not a claim that every PDF feature is implemented.

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
- Added checked numeric conversion calls to `Half`, `Float`, and `Double`, with compile-time finite/range checks for numeric literals.
- Added `Tensor<Element, [Dimension, ...]>` types with numeric element validation and static/symbolic dimension compatibility through signatures, semantic checking, HIR, and CLI example coverage.
- Improved declaration-level parser recovery so malformed function/record declarations do not consume later valid declarations; added a deterministic 1,000-input lexer/parser robustness smoke test.
- Added `start..end` integer range expressions with same-integer-type bounds, `Range<T>` HIR verification, and integer range iteration binding for `for` loops.
- Added nested `else if` parsing as nested conditional AST/HIR nodes, with return-path analysis across each branch.
- Added independent HIR verification that non-`Void` functions return on every supported conditional path.
- Aligned diagnostic carets with expanded tabs and UTF-8 scalar columns; terminal-specific wide/combining-character display widths remain out of scope.

## Current limitations

- Identifiers are ASCII-only. UTF-8 is validated in string contents, but Unicode identifier rules and full Unicode display-column mapping are not implemented.
- Numeric suffixes, raw strings, and the complete string escape repertoire remain incomplete.
- Semantic checking is a core subset; conversions require explicit checked mode and only basic static tensor shape equality is implemented. Numeric widening, tensor operations/broadcasting/device checks, exhaustive control-flow analysis, overloads, generics, module resolution, and complete diagnostics rendering remain.
- Parser coverage is still a subset: complete newline rules, AST printer snapshot tests, and full grammar recovery remain. Range endpoint inclusivity and runtime iteration behavior are not defined because this front end does not execute programs.
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
- All 11 CTest cases passed, including parser recovery/robustness, semantic analysis, HIR verification, diagnostics, and CLI checks for standard, record, and tensor-shape examples.
- The same 11 CTest cases now also verify nested `else if` return coverage, malformed HIR return-path rejection, and UTF-8/tab diagnostic caret alignment.
- The verification build was kept outside the project at `/tmp/indicode-progress-build`; no generated build files were added to the repository.