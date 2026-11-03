#pragma once

#include "indicode/source.hpp"

#include <string>

namespace indicode {

enum class TokenKind {
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    Keyword,
    Newline,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,
    Colon,
    Dot,
    Semicolon,
    Arrow,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Bang,
    Tilde,
    Equal,
    EqualEqual,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    AndAnd,
    OrOr,
    Invalid,
    EndOfFile
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    SourceSpan span;
};

std::string token_kind_name(TokenKind kind);

}