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

    std::cout << "Lexer tests passed\n";
}