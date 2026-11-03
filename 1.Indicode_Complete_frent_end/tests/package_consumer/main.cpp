#include "indicode/lexer.hpp"

int main() {
    const indicode::SourceFile source{"consumer.indi", "answer = 42\n"};
    const auto result = indicode::Lexer(source).lex();
    return result.diagnostics.empty() &&
                   !result.tokens.empty() &&
                   result.tokens.back().kind == indicode::TokenKind::EndOfFile
        ? 0
        : 1;
}
