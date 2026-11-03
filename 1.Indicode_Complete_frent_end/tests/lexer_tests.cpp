#include "indicode/lexer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}

int main() {
    const indicode::SourceFile source{
        "example.indi",
        "function add(a: Int, b: Int) -> Int {\r\n"
        "  total = a + b # comment\r\n"
        "  say(\"hello!\")\r\n"
        "}"
    };
    const auto result = indicode::Lexer(source).lex();

    require(result.diagnostics.empty(), "valid source should have no diagnostics");
    require(result.tokens.front().kind == indicode::TokenKind::Keyword,
            "function should be recognized as a keyword");
    require(result.tokens[1].lexeme == "add", "function name should remain an identifier");
    require(result.tokens[11].kind == indicode::TokenKind::Arrow,
            "function return arrow should be recognized");

    bool saw_newline = false;
    bool saw_string = false;
    for (const auto& token : result.tokens) {
        saw_newline = saw_newline || token.kind == indicode::TokenKind::Newline;
        saw_string = saw_string ||
                     (token.kind == indicode::TokenKind::StringLiteral && token.lexeme == "\"hello!\"");
    }
    require(saw_newline, "CRLF should produce newline separator tokens");
    require(saw_string, "quoted text should be emitted as one string token");
    require(result.tokens.back().kind == indicode::TokenKind::EndOfFile,
            "token stream should end with EOF");

    const indicode::SourceFile malformed{"bad.indi", "value = 1e+\ntext = \"open\n"};
    const auto errors = indicode::Lexer(malformed).lex();
    require(errors.diagnostics.size() == 2,
            "malformed exponent and unterminated string should both be diagnosed");
    require(errors.diagnostics[0].code == "E-LEX-004", "exponent diagnostic code should be stable");
    require(errors.diagnostics[1].code == "E-LEX-005", "string diagnostic code should be stable");

    const indicode::SourceFile unicode{
        "unicode.indi",
        "letter = \"\xC3\xA9\"\nface = \"\\u{1F600}\"\n"
    };
    const auto unicode_result = indicode::Lexer(unicode).lex();
    require(unicode_result.diagnostics.empty(), "valid UTF-8 and Unicode escapes should be accepted");

    const indicode::SourceFile invalid_utf8{"invalid_utf8.indi", "word = \"\xC3(\"\n"};
    const auto invalid_utf8_result = indicode::Lexer(invalid_utf8).lex();
    bool saw_utf8_error = false;
    for (const auto& diagnostic : invalid_utf8_result.diagnostics) {
        saw_utf8_error = saw_utf8_error || diagnostic.code == "E-LEX-006";
    }
    require(saw_utf8_error, "malformed UTF-8 in a string should be diagnosed");

    const indicode::SourceFile invalid_escape{"invalid_escape.indi", "word = \"\\q\"\n"};
    const auto invalid_escape_result = indicode::Lexer(invalid_escape).lex();
    require(!invalid_escape_result.diagnostics.empty() &&
                invalid_escape_result.diagnostics[0].code == "E-LEX-007",
            "unsupported string escapes should have a stable diagnostic");

        const indicode::SourceFile numeric_literals{
            "numbers.indi",
            "binary = 0b1010\noctal = 0o17\nhex = 0x2a\ngrouped = 1_000_000\n"
        };
        const auto numeric_result = indicode::Lexer(numeric_literals).lex();
        require(numeric_result.diagnostics.empty(),
                "binary, octal, hexadecimal, and separated integer literals should lex");

        const indicode::SourceFile malformed_numbers{
            "bad_numbers.indi",
            "bad_binary = 0b102\nbad_hex = 0x\nbad_separator = 12__3\n"
        };
        const auto malformed_number_result = indicode::Lexer(malformed_numbers).lex();
        require(malformed_number_result.diagnostics.size() == 3,
                "invalid radix digits, missing radix digits, and misplaced separators should be rejected");
        require(malformed_number_result.diagnostics[0].code == "E-LEX-009",
                "malformed numeric diagnostic should have a stable code");

        const indicode::SourceFile comments{
            "comments.indi",
            "first = 1 /* outer\n/* nested */ comment */ second = 2\n"
        };
        const auto comment_result = indicode::Lexer(comments).lex();
        require(comment_result.diagnostics.empty(), "nested block comments should be accepted");
        bool preserved_comment_newline = false;
        for (const auto& token : comment_result.tokens) {
            preserved_comment_newline = preserved_comment_newline ||
                                        (token.kind == indicode::TokenKind::Newline && token.span.start.line == 1);
        }
        require(preserved_comment_newline, "block-comment newlines should remain statement separators");

        const indicode::SourceFile open_comment{"open_comment.indi", "value = 1 /* never closed"};
        const auto open_comment_result = indicode::Lexer(open_comment).lex();
        require(open_comment_result.diagnostics.size() == 1 &&
                    open_comment_result.diagnostics[0].code == "E-LEX-010",
                "unterminated block comments should produce a recoverable diagnostic");

    std::cout << "Lexer tests passed\n";
}