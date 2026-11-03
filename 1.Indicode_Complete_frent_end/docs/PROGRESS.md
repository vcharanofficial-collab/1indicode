# Indicode Front-End Progress

Updated: 2026-11-03

## Scope

This phase builds only the Indicode front end, targeting a validated typed representation that a future LLVM backend can consume. It now includes an initial tensor type and static/symbolic shape-compatibility subset; tensor operations, broadcasting, dynamic shape guards, and device checks are not implemented. LLVM is not downloaded, vendored, or built in this repository. LLVM IR generation, machine-code generation, tensor execution, autodiff runtime/training, and device backends are outside the current phase.

## Production Readiness

This repository is a real, buildable, tested compiler-front-end prototype, but it is **not production-ready** and should not be represented as a deployable general-purpose compiler. The supported grammar and semantics are a small documented subset. There is no LLVM adapter or executable generation, language conformance suite, production CI/release process, fuzzing campaign, sanitizer gate, or compatibility/ABI policy. Use it for development and evaluation of the implemented subset, not as a production compiler.

## Completion

**Overall front-end completion: 95%**

The percentage measures progress toward the compiler front-end boundary described in the blueprint, not progress toward LLVM itself or the full compiler. It is a weighted estimate: each area's weight is fixed, and completion credit requires implemented, tested, documented acceptance behavior. Recent work adds installable CMake package consumption, stable local binding identities propagated through typed HIR, and a schema-versioned JSON serializer for verified HIR. The updated weighted total remains **95.00%, rounded to 95%**.

| Acceptance area | Weight | Area completion | Evidence / remaining work |
| --- | --- | --- |
| Project/build foundation | 5% | 95% | C++20, CMake, CTest, install/export package config, public headers and a separately configured/compiled package consumer; CI/release acceptance remains. |
| Source manager and diagnostics | 10% | 90% | Source spans, caret rendering aligned across tabs and UTF-8 scalar columns, JSON escaping, and fix-it edits with source spans; owned source manager, terminal display-width handling, broad fix-it coverage, and a stable versioned JSON schema remain. |
| Lexer | 15% | 92% | ASCII identifiers, UTF-8 source validation, strings/escapes, binary/octal/hex integers, digit separators, nested block comments with newline preservation, and scalar/tab-aware source columns; Unicode identifiers, suffix policy, trivia preservation, and complete numeric grammar remain. |
| Parser and AST | 15% | 95% | Functions, blocks, calls, precedence, typed lists, indexing, for-each, named arguments, records/member access, tensor element/shape types, integer ranges, nested `else if`, and a missing-parameter-colon fix-it; full grammar recovery and AST snapshots remain. |
| Name resolution | 10% | 90% | Lexical scopes, global/function/parameter/loop bindings, stable local symbol IDs, shadowing, and HIR revalidation of binding identity; modules/imports and stable function/type identities remain. |
| Type checking and semantic validation | 15% | 98% | Scalar/radix integer inference, integer and floating literal range checks, checked numeric conversions, record construction/field types, tensor element/shape compatibility, integer range bounds, indexing, nested conditional return analysis, loops and mutability; tensor operations, ownership/effects and full conversion policies remain. |
| Typed HIR and verification | 15% | 99% | Project-owned typed HIR carries stable local binding IDs, has a schema-versioned JSON serializer, and covers records, member access, tensor types, checked numeric conversions, collections, and integer ranges; verifier rechecks lexical visibility, type identity, immutability, return paths, and expression/call/loop contracts. Deserialization, compatibility policy, and broader language invariants remain. |
| CLI and diagnostics tooling | 5% | 95% | `check`, `tokens`, `ast`, `typed`, `typed-json`, source excerpts/carets aligned across tabs and UTF-8 scalar columns, and JSON diagnostics with fix-its; terminal-specific display widths, editor interfaces, and stable diagnostics/HIR compatibility policies remain. |
| Tests, docs, and integration | 10% | 99% | Fourteen CTest cases cover parser recovery/fix-its, symbol identity/scope/mutability in HIR, schema-versioned HIR JSON, nested `else if` return paths, float literal bounds, deterministic malformed-input smoke testing, tensor shapes, diagnostics, CLI, and an installed-package consumer; full conformance, coverage-guided fuzzing, sanitizers and CI remain. |

Updated weighted estimate: 4.75 + 9.0 + 13.8 + 14.25 + 9.0 + 14.7 + 14.85 + 4.75 + 9.9 = **95.00%, rounded to 95%**. This is a judgment-based estimate for the implemented front-end boundary, not a claim that 95% of a complete language specification or all possible compiler work is done.

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
- Strengthened independent HIR checks to reject duplicate parameter names, argument-label metadata that is not aligned with call operands, and returns outside functions.
- Aligned diagnostic carets with expanded tabs and UTF-8 scalar columns; terminal-specific wide/combining-character display widths remain out of scope.
- Added optional diagnostic fix-its with replacement spans, messages, text hints, and JSON output; the parser suggests inserting `: ` after a function parameter name when its colon is missing.
- Added semantic errors for decimal floating-point literals that overflow or underflow the supported host conversion range.
- Lexer source columns now count valid UTF-8 continuation bytes as part of one scalar and expand tabs to four-column stops; byte offsets remain unchanged for slicing and edits.
- The lexer rejects malformed UTF-8 in line and block comments as well as strings and other source tokens.
- Added stable IDs for local variables, parameters, and loop bindings through semantic analysis and AST-to-HIR lowering; HIR printing exposes those IDs for backend inspection.
- HIR verification now independently checks lexical name visibility, matching symbol identity and type, immutable assignments, duplicate declarations, loop scope boundaries, and global bindings visible from functions.
- Added CMake install/export rules, a versioned package config, and a CTest consumer that installs the library, configures a separate CMake project, links `indicode::frontend`, and runs it.
- Added canonical `typed-json` CLI output with schema version 1, typed nodes, source spans, argument labels, and stable local symbol IDs; a CMake JSON check validates the schema and a parameter reference.

## Current limitations

- Identifiers are ASCII-only. UTF-8 source sequences are validated, but Unicode identifier rules and full Unicode display-column mapping are not implemented.
- Numeric suffixes, raw strings, and the complete string escape repertoire remain incomplete.
- Semantic checking is a core subset; conversions require explicit checked mode and only basic static tensor shape equality is implemented. Numeric widening, tensor operations/broadcasting/device checks, exhaustive control-flow analysis, overloads, generics, module resolution, and complete diagnostics rendering remain.
- Parser coverage is still a subset: complete newline rules, AST printer snapshot tests, and full grammar recovery remain. Range endpoint inclusivity and runtime iteration behavior are not defined because this front end does not execute programs.
- Typed HIR/verifier currently cover the existing language subset only; they need complete semantic coverage, stable function/type identities, deserialization, a compatibility policy, and conformance tests before an LLVM adapter is ready.

## Milestones

1. **Source model and lexer:** complete UTF-8/source management, token coverage, literal validation, and lexer tests.
2. **Parser and AST:** implement the documented core grammar, precedence, source spans, AST printer, and recovery tests.
3. **Names and types:** add scopes, symbol resolution, core types, inference, conversion/range checks, and diagnostics.
4. **Core semantic validation:** add function/control-flow checks, mutability rules, and negative tests.
5. **Typed HIR:** initial lowering, printer, verifier, schema-versioned JSON serialization, and tests are implemented; complete semantics, deserialization, compatibility policy, and robust invariants remain.
6. **Front-end CLI and release checks:** `check`, `tokens`, `ast`, `typed`, `typed-json`, and JSON diagnostics exist; stable schema/compatibility policies, full integration/conformance tests, fuzzing, and release checks remain.

LLVM/backend work begins only after these front-end milestones are complete and accepted. The front end is being designed to connect to LLVM later without requiring the LLVM framework to be downloaded into this repository now.

## Latest verification

- A fresh Release-mode out-of-tree CMake configure/build succeeded with GCC 13.3 in `/tmp/indicode-full-verify-build`.
- All 14 CTest cases pass, including typed-HIR JSON inspection and the separately built installed-package consumer.
- The suite also verifies nested `else if` return coverage, malformed HIR return-path rejection, invalid HIR parameter/call/return/symbol-scope invariants, UTF-8/tab diagnostic caret alignment, and float literal overflow/underflow rejection.
- Lexer regression tests confirm that UTF-8 byte offsets and human-facing source columns remain distinct and consistent after tabs and non-ASCII string contents.
- The verification build was kept outside the project; no generated build files were added to the repository.