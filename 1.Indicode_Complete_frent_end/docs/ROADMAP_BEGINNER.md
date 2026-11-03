# Indicode Front-End Roadmap for Beginners

Updated: 2026-11-03

## What Are We Building?

We are building the part of a compiler that reads Indicode source code and checks what it means. It should eventually turn a correct program into a typed, verified Indicode representation that a separately installed LLVM backend can consume.

LLVM is a large, separate compiler framework. This project does **not** download, copy, or build LLVM. Our front end and its tests must keep working without LLVM. Connecting an optional backend is a later project stage, after the front end has a stable typed representation.

The PDF is a blueprint, not a list of features already completed. Its 777 concepts are a long-term catalogue, not 777 built-in types or a promise to implement every idea immediately.

## Current Overall Progress: 80%

This percentage means estimated completion of the **front-end work needed before a future LLVM adapter**. It does not mean 80% of LLVM, machine-code generation, a GPU compiler, or the AI runtime is complete.

The estimate uses the weighted acceptance areas in `PROGRESS.md`:

- Previous estimate: 69.0%.
- Records, named record construction, member type checking, checked integer conversions, and HIR verification were added with tests and an end-to-end CLI example.
- Updated weighted estimate: 80.05%, rounded to **80%** (see `PROGRESS.md`).

This is an estimate, not a marketing score. It may go down if a review finds missing acceptance behavior. Passing tests prove only the tested cases, not completeness of the language.

## What Already Works?

- C++20/CMake builds the front-end without LLVM.
- The lexer splits a small subset of Indicode text into tokens.
- The parser builds an AST (Abstract Syntax Tree: a tree-shaped record of the code's structure).
- Semantic checks catch selected name, type, mutability, function-call, return, and integer-range errors.
- The compiler lowers checked AST into project-owned typed HIR (High-level Intermediate Representation: a simpler, explicitly typed form of the program).
- The HIR verifier checks required type annotations, expression/operator types, function calls, and return types.
- Lists can be created, indexed, and iterated with `for`; text can also be iterated/indexed as `Char` values.
- String contents are UTF-8 validated; supported escapes include Unicode scalar escapes such as `\u{e9}`. A single Unicode scalar infers `Char`, more infer `Text`.
- `indicode check`, `tokens`, `ast`, `typed`, and opt-in `--json-diagnostics` work for the current subset.
- Integer literals support `0b` binary, `0o` octal, `0x` hexadecimal, and `_` digit separators; malformed forms are rejected.
- Function calls accept named arguments such as `add(b: 22, a: 20)` and reject unknown, duplicate, missing, or out-of-order arguments.
- Nested block comments use `/* ... */`; newlines inside them still separate statements. An unclosed comment is a compile-time error.
- Records support typed fields, construction with named fields, and read-only member access with type checking.
- Integer values can be explicitly converted with functions such as `to_Int2(value, checked: true)`. Integer constants outside the destination range are rejected; dynamic inputs require the checked mode.
- Nine CTest cases cover the current front-end behavior, including the record sample.

## Newly Supported Syntax

Records declare fields with explicit types. Construction uses named arguments and must provide every field exactly once.

```indi
record Point {
    x: Int,
    y: Int
}

origin = Point(x: 20, y: 22)
say(origin.x)
```

Unknown/missing fields, duplicate field declarations, and mismatched field types are errors. Record methods, field assignment, and visibility rules are not implemented.

Integer conversion is explicit and checked:

```indi
small = to_Int2(30000, checked: true)
```

The current conversion subset targets integer types only. Floating-point conversion rules and runtime execution of conversions are not implemented.

Lists are written with square brackets. A non-empty list must contain values of one type. Its type is inferred from those values.

```indi
items = [20, 22]
answer = items[0]

for item in items {
	say(item)
}
```

An index must be an `Int`, and the collection type must support indexing. This front end checks types only; it does not execute programs or perform runtime bounds checks. Text indexing and iteration produce one `Char` Unicode scalar at a time.

```indi
accent = "\u{e9}"  # one Unicode scalar, so this is Char
message = "hello"   # multiple scalars, so this is Text
for letter in message {
	say(letter)
}

answer = add(b: items[1], a: items[0])
mask = 0b1111
population = 1_000
```

Invalid cases are rejected with stable diagnostics:

```indi
mixed = [1, "two"]       # E-SEMA-025: list elements need one type
wrong_index = items[true] # E-SEMA-026: index must be Int
for value in 5 { say(value) } # E-SEMA-023: Int is not iterable
bad_text = "\q"           # E-LEX-007: unsupported string escape
bad_binary = 0b102         # E-LEX-009: invalid binary digit
bad_names = add(c: 1, a: 2) # E-SEMA-028: unknown named argument
```

An unclosed block comment is also an error:

```indi
value = 1 /* comment never ends
```

The current grammar does not yet provide a type annotation syntax for an empty list, so `[]` is rejected until that syntax is designed.

## Roadmap

| Stage | Status | In plain language | Completion evidence |
| --- | --- | --- | --- |
| 1. Project foundation | Mostly done | Build the project without large external frameworks. | C++20/CMake build and CTest work without LLVM or network downloads. |
| 2. Source and lexer | Started | Read characters and recognize names, numbers, strings, punctuation, and errors. | Unicode identifiers, numeric suffixes, raw/block comments, and full source display-column mapping. |
| 3. Parser and AST | Started | Recognize the language's grammar and record its structure. | Ranges, complete newline rules, AST golden tests, full recovery. |
| 4. Names and types | Started | Decide what names refer to and whether operations use compatible types. | Modules, imports, conversions, overflow, ownership/effects, tensor shapes, and negative diagnostics. |
| 5. Typed HIR | **Started; more core syntax implemented** | Convert checked source into our own typed representation. | More language coverage, stable IDs, effects/conversions, verifier invariants, serialization/versioning. |
| 6. Diagnostics and tools | Started | Make errors useful to people and editors. | JSON schema stability, fix-its, editor interfaces, and complete recovery diagnostics. |
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
/tmp/indicode-build/indicode check examples/records.indi
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
