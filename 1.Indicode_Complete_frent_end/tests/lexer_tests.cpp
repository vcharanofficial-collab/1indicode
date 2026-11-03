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

    std::cout << "Lexer tests passed\n";
}