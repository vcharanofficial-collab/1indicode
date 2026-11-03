#include "indicode/lexer.hpp"

#include <array>
#include <utility>

namespace indicode {
namespace {

bool is_digit(char value) {
    return value >= '0' && value <= '9';
}

bool is_alpha(char value) {
    return (value >= 'a' && value <= 'z') ||
           (value >= 'A' && value <= 'Z') || value == '_';
}

bool is_alphanumeric(char value) {
    return is_alpha(value) || is_digit(value);
}

bool is_keyword(const std::string& value) {
    static constexpr std::array keywords{
        "function", "return", "if", "else", "repeat", "times", "for", "in",
        "while", "let", "var", "true", "false"
    };
    for (const auto* keyword : keywords) {
        if (value == keyword) {
            return true;
        }
    }
    return false;
}

}

std::string token_kind_name(TokenKind kind) {
    switch (kind) {
    case TokenKind::Identifier: return "Identifier";
    case TokenKind::IntegerLiteral: return "IntegerLiteral";
    case TokenKind::FloatLiteral: return "FloatLiteral";
    case TokenKind::StringLiteral: return "StringLiteral";
    case TokenKind::Keyword: return "Keyword";
    case TokenKind::Newline: return "Newline";
    case TokenKind::LeftParen: return "LeftParen";
    case TokenKind::RightParen: return "RightParen";
    case TokenKind::LeftBrace: return "LeftBrace";
    case TokenKind::RightBrace: return "RightBrace";
    case TokenKind::LeftBracket: return "LeftBracket";
    case TokenKind::RightBracket: return "RightBracket";
    case TokenKind::Comma: return "Comma";
    case TokenKind::Colon: return "Colon";
    case TokenKind::Dot: return "Dot";
    case TokenKind::Semicolon: return "Semicolon";
    case TokenKind::Arrow: return "Arrow";
    case TokenKind::Plus: return "Plus";
    case TokenKind::Minus: return "Minus";
    case TokenKind::Star: return "Star";
    case TokenKind::Slash: return "Slash";
    case TokenKind::Percent: return "Percent";
    case TokenKind::Bang: return "Bang";
    case TokenKind::Tilde: return "Tilde";
    case TokenKind::Equal: return "Equal";
    case TokenKind::EqualEqual: return "EqualEqual";
    case TokenKind::BangEqual: return "BangEqual";
    case TokenKind::Less: return "Less";
    case TokenKind::LessEqual: return "LessEqual";
    case TokenKind::Greater: return "Greater";
    case TokenKind::GreaterEqual: return "GreaterEqual";
    case TokenKind::AndAnd: return "AndAnd";
    case TokenKind::OrOr: return "OrOr";
    case TokenKind::Invalid: return "Invalid";
    case TokenKind::EndOfFile: return "EndOfFile";
    }
    return "Unknown";
}

Lexer::Lexer(const SourceFile& source) : source_(source) {}

LexResult Lexer::lex() {
    while (!at_end()) {
        scan_token();
    }

    const auto eof_location = location();
    result_.tokens.push_back({TokenKind::EndOfFile, "", {eof_location, eof_location}});
    return std::move(result_);
}

bool Lexer::at_end() const {
    return offset_ >= source_.contents.size();
}

char Lexer::current() const {
    return at_end() ? '\0' : source_.contents[offset_];
}

char Lexer::peek(std::size_t distance) const {
    const auto next = offset_ + distance;
    return next >= source_.contents.size() ? '\0' : source_.contents[next];
}

char Lexer::advance() {
    const char value = current();
    ++offset_;
    ++column_;
    return value;
}

bool Lexer::match(char expected) {
    if (at_end() || current() != expected) {
        return false;
    }
    advance();
    return true;
}

SourceLocation Lexer::location() const {
    return {offset_, line_, column_};
}

void Lexer::add_token(TokenKind kind, SourceLocation start) {
    const auto end = location();
    result_.tokens.push_back({
        kind,
        source_.contents.substr(start.offset, end.offset - start.offset),
        {start, end}
    });
}

void Lexer::add_error(std::string code, std::string message, SourceLocation start) {
    const auto end = location();
    result_.diagnostics.push_back({DiagnosticSeverity::Error, std::move(code),
                                   std::move(message), {start, end}});
}

void Lexer::scan_token() {
    const auto start = location();
    const char value = advance();

    switch (value) {
    case ' ':
    case '\t':
    case '\f':
        return;
    case '\r':
        if (current() == '\n') {
            ++offset_;
        }
        ++line_;
        column_ = 1;
        result_.tokens.push_back({TokenKind::Newline, "\n", {start, location()}});
        return;
    case '\n':
        ++line_;
        column_ = 1;
        result_.tokens.push_back({TokenKind::Newline, "\n", {start, location()}});
        return;
    case '#':
        while (!at_end() && current() != '\n' && current() != '\r') {
            advance();
        }
        return;
    case '(': add_token(TokenKind::LeftParen, start); return;
    case ')': add_token(TokenKind::RightParen, start); return;
    case '{': add_token(TokenKind::LeftBrace, start); return;
    case '}': add_token(TokenKind::RightBrace, start); return;
    case '[': add_token(TokenKind::LeftBracket, start); return;
    case ']': add_token(TokenKind::RightBracket, start); return;
    case ',': add_token(TokenKind::Comma, start); return;
    case ':': add_token(TokenKind::Colon, start); return;
    case '.': add_token(TokenKind::Dot, start); return;
    case ';': add_token(TokenKind::Semicolon, start); return;
    case '+': add_token(TokenKind::Plus, start); return;
    case '*': add_token(TokenKind::Star, start); return;
    case '/': add_token(TokenKind::Slash, start); return;
    case '%': add_token(TokenKind::Percent, start); return;
    case '~': add_token(TokenKind::Tilde, start); return;
    case '-': add_token(match('>') ? TokenKind::Arrow : TokenKind::Minus, start); return;
    case '!': add_token(match('=') ? TokenKind::BangEqual : TokenKind::Bang, start); return;
    case '=': add_token(match('=') ? TokenKind::EqualEqual : TokenKind::Equal, start); return;
    case '<': add_token(match('=') ? TokenKind::LessEqual : TokenKind::Less, start); return;
    case '>': add_token(match('=') ? TokenKind::GreaterEqual : TokenKind::Greater, start); return;
    case '&':
        if (match('&')) {
            add_token(TokenKind::AndAnd, start);
        } else {
            add_error("E-LEX-001", "expected '&' after '&'", start);
            add_token(TokenKind::Invalid, start);
        }
        return;
    case '|':
        if (match('|')) {
            add_token(TokenKind::OrOr, start);
        } else {
            add_error("E-LEX-002", "expected '|' after '|'", start);
            add_token(TokenKind::Invalid, start);
        }
        return;
    case '"': scan_string(start); return;
    default:
        if (is_alpha(value)) {
            scan_identifier(start);
        } else if (is_digit(value)) {
            scan_number(start);
        } else {
            add_error("E-LEX-003", "unrecognized character", start);
            add_token(TokenKind::Invalid, start);
        }
        return;
    }
}

void Lexer::scan_identifier(SourceLocation start) {
    while (is_alphanumeric(current())) {
        advance();
    }
    const auto lexeme = source_.contents.substr(start.offset, offset_ - start.offset);
    add_token(is_keyword(lexeme) ? TokenKind::Keyword : TokenKind::Identifier, start);
}

void Lexer::scan_number(SourceLocation start) {
    while (is_digit(current())) {
        advance();
    }

    bool is_float = false;
    if (current() == '.' && is_digit(peek())) {
        is_float = true;
        advance();
        while (is_digit(current())) {
            advance();
        }
    }

    if (current() == 'e' || current() == 'E') {
        is_float = true;
        advance();
        if (current() == '+' || current() == '-') {
            advance();
        }
        if (!is_digit(current())) {
            add_error("E-LEX-004", "expected a digit in numeric exponent", start);
            add_token(TokenKind::Invalid, start);
            return;
        }
        while (is_digit(current())) {
            advance();
        }
    }

    add_token(is_float ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral, start);
}

void Lexer::scan_string(SourceLocation start) {
    bool terminated = false;
    while (!at_end()) {
        if (current() == '"') {
            advance();
            terminated = true;
            break;
        }
        if (current() == '\n' || current() == '\r') {
            break;
        }
        if (current() == '\\') {
            advance();
            if (!at_end()) {
                advance();
            }
        } else {
            advance();
        }
    }

    if (!terminated) {
        add_error("E-LEX-005", "unterminated string literal", start);
        add_token(TokenKind::Invalid, start);
        return;
    }
    add_token(TokenKind::StringLiteral, start);
}

}