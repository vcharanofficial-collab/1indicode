#pragma once

#include "indicode/ast.hpp"
#include "indicode/diagnostic.hpp"
#include "indicode/token.hpp"

#include <vector>

namespace indicode {

struct ParseResult {
    Program program;
    std::vector<Diagnostic> diagnostics;
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    ParseResult parse();

private:
    const std::vector<Token>& tokens_;
    ParseResult result_;
    std::size_t current_ = 0;

    const Token& peek() const;
    const Token& previous() const;
    bool at_end() const;
    bool check(TokenKind kind) const;
    bool check_keyword(const char* keyword) const;
    const Token& advance();
    bool match(TokenKind kind);
    bool match_keyword(const char* keyword);
    const Token* consume(TokenKind kind, const char* message);
    const Token* consume_identifier(const char* message);
    void error(const Token& token, const char* message);
    void skip_separators();
    void synchronize();
    void declaration();
    bool function_declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> variable_statement(bool mutable_binding, const Token* type_token = nullptr);
    std::unique_ptr<Stmt> return_statement(const Token& keyword);
    std::unique_ptr<Stmt> if_statement(const Token& keyword);
    std::unique_ptr<Stmt> while_statement(const Token& keyword);
    std::unique_ptr<Stmt> repeat_statement(const Token& keyword);
    bool block(std::vector<std::unique_ptr<Stmt>>& statements);
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> parse_precedence(int minimum_precedence);
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> finish_call(std::unique_ptr<Expr> callee);
    static int precedence(TokenKind kind);
};

}