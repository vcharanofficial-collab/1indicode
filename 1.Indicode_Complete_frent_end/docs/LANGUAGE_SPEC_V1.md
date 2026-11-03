# Indicode Front-End Language Specification

**Grammar version:** 1.0.0  
**Status:** Normative for the syntax accepted by this front-end release.

This specification defines the source syntax parsed by the C++20 front end. It
does not define execution behavior, code generation, or runtime behavior.
Changes to accepted syntax require a grammar-version change and corresponding
parser, lexer, and conformance-test updates. Bug fixes that reject malformed
source or accept syntax already described here do not change the grammar
version.

## Source and tokens

Source is UTF-8. Invalid UTF-8 is diagnosed. Identifiers are ASCII and have the
form `[A-Za-z_][A-Za-z0-9_]*`. The reserved words are:

`function record return if else repeat times for in while let var true false`

Integer literals are decimal, binary (`0b`), octal (`0o`), or hexadecimal
(`0x`), with underscores permitted only between valid digits. Decimal floating
literals have digits after a decimal point and/or a decimal exponent. A string is a
double-quoted, single-line sequence supporting `\"`, `\\`, `\n`, `\r`, `\t`,
`\0`, and Unicode scalar escapes `\u{...}`. Numeric suffixes, raw strings,
Unicode identifiers, and multiline strings are not part of v1.

Punctuation and operators are `(` `)` `{` `}` `[` `]` `,` `:` `.` `..` `;`
`->` `+` `-` `*` `/` `%` `!` `~` `=` `==` `!=` `<` `<=` `>` `>=` `&&` and
`||`. `#` line comments and nested `/* ... */` block comments are trivia.
Newlines remain syntax separators, including newlines inside block comments.

## Grammar

The notation below uses `*` for zero or more, `+` for one or more, and `?`
for optional. Newline and semicolon are statement separators; repeated
separators are allowed between declarations and statements.

```ebnf
program          = { separator }, { declaration, { separator } }, EOF ;
declaration      = record-declaration | function-declaration | statement ;
record-declaration
                 = "record", identifier, "{", { separator },
                   [ field, { (",", { separator } | separator), field }, [","], { separator } ],
                   "}" ;
field            = identifier, ":", type ;
function-declaration
                 = "function", identifier, "(", [ parameter, { ",", parameter } ], ")",
                   [ "->", type ], block ;
parameter        = identifier, ":", type ;
type             = "Tensor", "<", type, ",", "[", dimension, { ",", dimension }, "]", ">"
                 | identifier, [ "<", type, ">" ] ;
dimension        = identifier | integer-literal ;

block            = "{", { separator }, { statement, { separator } }, "}" ;
statement        = "let", identifier, [ ":", type ], "=", expression
                 | "var", identifier, [ ":", type ], "=", expression
                 | type, identifier, "=", expression
                 | identifier, "=", expression
                 | "return", [ expression ]
                 | conditional
                 | "while", expression, block
                 | "repeat", expression, "times", block
                 | "for", identifier, "in", expression, block
                 | expression ;
conditional      = "if", expression, block, [ "else", (block | conditional) ] ;
expression       = prefix-expression, { binary-operator, prefix-expression } ;
prefix-expression
                 = ( "!" | "+" | "-" | "~" ), prefix-expression
                 | postfix-expression ;
postfix-expression
                 = primary, { index | member } ;
primary          = identifier, [ call ] | integer-literal | float-literal | string-literal
                 | "true" | "false" | list-literal | "(", expression, ")" ;
list-literal     = "[", [ expression, { ",", expression } ], "]" ;
call             = "(", [ positional-arguments, [ ",", named-arguments ]
                   | named-arguments ], ")" ;
positional-arguments
                 = expression, { ",", expression } ;
named-arguments  = named-argument, { ",", named-argument } ;
named-argument   = identifier, ":", expression ;
index            = "[", expression, "]" ;
member           = ".", identifier ;
binary-operator  = "||" | "&&" | ".." | "==" | "!=" | "<" | "<=" | ">" | ">="
                 | "+" | "-" | "*" | "/" | "%" ;
separator        = newline | ";" ;
```

The parser rejects positional call arguments after a named argument. Calls and
record construction use the same syntax; semantic analysis resolves the name.
The `if` production is recursively nestable, so any-length `else if` chains
are accepted. The legacy `identifier identifier = expression` declaration
form is the non-generic `type identifier` alternative above.

Modules/imports, generic function or record declarations, overloads, and
declaration visibility are explicitly outside grammar v1. `List<T>` is the
single-argument generic type spelling accepted by the parser; tensor types use
the dedicated shape form shown above.

## Expression precedence and associativity

All binary operators are left-associative. A larger number binds more tightly.
Calls, indexing, and member access are postfix forms and bind more tightly
than prefix operators.

| Precedence | Operators | Associativity |
|---:|---|---|
| 1 | `||` | left |
| 2 | `&&` | left |
| 3 | `..` | left |
| 4 | `==`, `!=` | left |
| 5 | `<`, `<=`, `>`, `>=` | left |
| 6 | `+`, `-` | left |
| 7 | `*`, `/`, `%` | left |

Prefix `!`, `+`, `-`, and `~` are right-nested. Parentheses override precedence.

## Scope and compatibility

This version freezes syntax only. Type rules, diagnostics, symbol resolution,
and HIR compatibility have separate acceptance requirements and are not
implied by a successful parse. A future incompatible syntax change must update
this version, the public `grammar_version` constant, examples, and tests
together.
