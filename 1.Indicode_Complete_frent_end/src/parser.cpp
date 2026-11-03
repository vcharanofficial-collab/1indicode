#include "indicode/parser.hpp"

#include <algorithm>
#include <utility>

namespace indicode {

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

ParseResult Parser::parse() {
    skip_separators();
    while (!at_end()) {
        const auto start = current_;
        declaration();
        if (current_ == start) {
            advance();
        }
        skip_separators();
    }
    return std::move(result_);
}

const Token& Parser::peek() const {
    if (tokens_.empty()) {
        static const Token eof{TokenKind::EndOfFile, "", {{0, 1, 1}, {0, 1, 1}}};
        return eof;
    }
    return tokens_[std::min(current_, tokens_.size() - 1)];
}

const Token& Parser::previous() const {
    if (tokens_.empty() || current_ == 0) {
        return peek();
    }
    return tokens_[current_ - 1];
}

bool Parser::at_end() const {
    return check(TokenKind::EndOfFile);
}

bool Parser::check(TokenKind kind) const {
    return peek().kind == kind;
}

bool Parser::check_keyword(const char* keyword) const {
    return check(TokenKind::Keyword) && peek().lexeme == keyword;
}

const Token& Parser::advance() {
    if (!at_end()) {
        ++current_;
    }
    return previous();
}

bool Parser::match(TokenKind kind) {
    if (!check(kind)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::match_keyword(const char* keyword) {
    if (!check_keyword(keyword)) {
        return false;
    }
    advance();
    return true;
}

const Token* Parser::consume(TokenKind kind, const char* message) {
    if (check(kind)) {
        return &advance();
    }
    error(peek(), message);
    return nullptr;
}

const Token* Parser::consume_identifier(const char* message) {
    return consume(TokenKind::Identifier, message);
}

void Parser::error(const Token& token, const char* message) {
    result_.diagnostics.push_back({DiagnosticSeverity::Error, "E-PARSE-001", message, token.span});
}

void Parser::skip_separators() {
    while (match(TokenKind::Newline) || match(TokenKind::Semicolon)) {
    }
}

void Parser::synchronize() {
    while (!at_end() && !check(TokenKind::RightBrace)) {
        if (previous().kind == TokenKind::Newline || previous().kind == TokenKind::Semicolon) {
            return;
        }
        if (check_keyword("function") || check_keyword("if") || check_keyword("return")) {
            return;
        }
        advance();
    }
}

void Parser::declaration() {
    if (match_keyword("function")) {
        if (!function_declaration()) {
            synchronize();
        }
        return;
    }

    auto parsed = statement();
    if (parsed) {
        result_.program.statements.push_back(std::move(parsed));
    } else {
        synchronize();
    }
}

bool Parser::function_declaration() {
    const auto& function_keyword = previous();
    const auto* name = consume_identifier("expected function name");
    if (name == nullptr || consume(TokenKind::LeftParen, "expected '(' after function name") == nullptr) {
        return false;
    }

    FunctionDecl function;
    function.name = name->lexeme;
    function.span.start = function_keyword.span.start;

    if (!check(TokenKind::RightParen)) {
        do {
            const auto* parameter_name = consume_identifier("expected parameter name");
            if (parameter_name == nullptr || consume(TokenKind::Colon, "expected ':' after parameter name") == nullptr) {
                return false;
            }
            const auto* parameter_type = consume_identifier("expected parameter type");
            if (parameter_type == nullptr) {
                return false;
            }
            function.parameters.push_back({parameter_name->lexeme, parameter_type->lexeme,
                                           {parameter_name->span.start, parameter_type->span.end}});
        } while (match(TokenKind::Comma));
    }

    if (consume(TokenKind::RightParen, "expected ')' after parameters") == nullptr) {
        return false;
    }
    if (match(TokenKind::Arrow)) {
        const auto* return_type = consume_identifier("expected return type after '->'");
        if (return_type == nullptr) {
            return false;
        }
        function.return_type = return_type->lexeme;
    }
    if (consume(TokenKind::LeftBrace, "expected '{' before function body") == nullptr) {
        return false;
    }
    if (!block(function.body)) {
        return false;
    }
    function.span.end = previous().span.end;
    result_.program.functions.push_back(std::move(function));
    return true;
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match_keyword("let")) {
        return variable_statement(false);
    }
    if (match_keyword("var")) {
        return variable_statement(true);
    }
    if (match_keyword("return")) {
        return return_statement(previous());
    }
    if (match_keyword("if")) {
        return if_statement(previous());
    }
    if (match_keyword("while")) {
        return while_statement(previous());
    }
    if (match_keyword("repeat")) {
        return repeat_statement(previous());
    }
    if (match_keyword("for")) {
        return for_each_statement(previous());
    }
    if (match(TokenKind::Identifier)) {
        const auto identifier = previous();
        if (check(TokenKind::Identifier)) {
            --current_;
            const auto type = advance();
            return variable_statement(true, &type);
        }
        if (match(TokenKind::Equal)) {
            auto result = std::make_unique<Stmt>();
            result->kind = StmtKind::Assignment;
            result->name = identifier.lexeme;
            result->span.start = identifier.span.start;
            result->expression = expression();
            if (!result->expression) {
                return nullptr;
            }
            result->span.end = result->expression->span.end;
            return result;
        }
        --current_;
    }

    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::Expression;
    result->expression = expression();
    if (!result->expression) {
        return nullptr;
    }
    result->span = result->expression->span;
    return result;
}

std::unique_ptr<Stmt> Parser::variable_statement(bool mutable_binding, const Token* type_token) {
    const auto* name = consume_identifier("expected variable name");
    if (name == nullptr) {
        return nullptr;
    }

    std::string declared_type;
    if (match(TokenKind::Colon)) {
        const auto* type = consume_identifier("expected type after ':'");
        if (type == nullptr) {
            return nullptr;
        }
        declared_type = type->lexeme;
    } else if (type_token != nullptr) {
        declared_type = type_token->lexeme;
    }

    if (consume(TokenKind::Equal, "expected '=' before variable initializer") == nullptr) {
        return nullptr;
    }
    auto initializer = expression();
    if (!initializer) {
        return nullptr;
    }

    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::Variable;
    result->name = name->lexeme;
    result->declared_type = std::move(declared_type);
    result->mutable_binding = mutable_binding;
    result->span = {name->span.start, initializer->span.end};
    result->expression = std::move(initializer);
    return result;
}

std::unique_ptr<Stmt> Parser::return_statement(const Token& keyword) {
    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::Return;
    result->span.start = keyword.span.start;
    if (!check(TokenKind::Newline) && !check(TokenKind::Semicolon) &&
        !check(TokenKind::RightBrace) && !at_end()) {
        result->expression = expression();
        if (!result->expression) {
            return nullptr;
        }
        result->span.end = result->expression->span.end;
    } else {
        result->span.end = keyword.span.end;
    }
    return result;
}

std::unique_ptr<Stmt> Parser::if_statement(const Token& keyword) {
    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::If;
    result->span.start = keyword.span.start;
    result->expression = expression();
    if (!result->expression || consume(TokenKind::LeftBrace, "expected '{' after if condition") == nullptr ||
        !block(result->body)) {
        return nullptr;
    }
    if (match_keyword("else")) {
        if (consume(TokenKind::LeftBrace, "expected '{' after else") == nullptr || !block(result->else_body)) {
            return nullptr;
        }
    }
    result->span.end = previous().span.end;
    return result;
}

std::unique_ptr<Stmt> Parser::while_statement(const Token& keyword) {
    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::While;
    result->span.start = keyword.span.start;
    result->expression = expression();
    if (!result->expression || consume(TokenKind::LeftBrace, "expected '{' after while condition") == nullptr ||
        !block(result->body)) {
        return nullptr;
    }
    result->span.end = previous().span.end;
    return result;
}

std::unique_ptr<Stmt> Parser::repeat_statement(const Token& keyword) {
    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::Repeat;
    result->span.start = keyword.span.start;
    result->expression = expression();
    if (!result->expression || !match_keyword("times")) {
        error(peek(), "expected 'times' after repeat count");
        return nullptr;
    }
    if (consume(TokenKind::LeftBrace, "expected '{' after repeat count") == nullptr || !block(result->body)) {
        return nullptr;
    }
    result->span.end = previous().span.end;
    return result;
}

std::unique_ptr<Stmt> Parser::for_each_statement(const Token& keyword) {
    const auto* name = consume_identifier("expected loop variable after 'for'");
    if (name == nullptr || !match_keyword("in")) {
        error(peek(), "expected 'in' after loop variable");
        return nullptr;
    }
    auto iterable = expression();
    if (!iterable || consume(TokenKind::LeftBrace, "expected '{' after for-each iterable") == nullptr) {
        return nullptr;
    }

    auto result = std::make_unique<Stmt>();
    result->kind = StmtKind::ForEach;
    result->name = name->lexeme;
    result->span.start = keyword.span.start;
    result->expression = std::move(iterable);
    if (!block(result->body)) {
        return nullptr;
    }
    result->span.end = previous().span.end;
    return result;
}

bool Parser::block(std::vector<std::unique_ptr<Stmt>>& statements) {
    skip_separators();
    while (!check(TokenKind::RightBrace) && !at_end()) {
        const auto start = current_;
        auto parsed = statement();
        if (parsed) {
            statements.push_back(std::move(parsed));
        } else {
            synchronize();
        }
        if (current_ == start) {
            advance();
        }
        skip_separators();
    }
    return consume(TokenKind::RightBrace, "expected '}' after block") != nullptr;
}

std::unique_ptr<Expr> Parser::expression() {
    return parse_precedence(1);
}

std::unique_ptr<Expr> Parser::parse_precedence(int minimum_precedence) {
    auto left = unary();
    if (!left) {
        return nullptr;
    }

    while (precedence(peek().kind) >= minimum_precedence) {
        const auto operation = advance();
        const auto operation_precedence = precedence(operation.kind);
        auto right = parse_precedence(operation_precedence + 1);
        if (!right) {
            return nullptr;
        }
        auto combined = std::make_unique<Expr>();
        combined->kind = ExprKind::Binary;
        combined->value = operation.lexeme;
        combined->span = {left->span.start, right->span.end};
        combined->left = std::move(left);
        combined->right = std::move(right);
        left = std::move(combined);
    }
    return left;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenKind::Bang) || match(TokenKind::Minus) || match(TokenKind::Plus) || match(TokenKind::Tilde)) {
        const auto operation = previous();
        auto operand = unary();
        if (!operand) {
            return nullptr;
        }
        auto result = std::make_unique<Expr>();
        result->kind = ExprKind::Unary;
        result->value = operation.lexeme;
        result->span = {operation.span.start, operand->span.end};
        result->left = std::move(operand);
        return result;
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenKind::IntegerLiteral) || match(TokenKind::FloatLiteral) ||
        match(TokenKind::StringLiteral) || match(TokenKind::Keyword)) {
        const auto& token = previous();
        if (token.kind == TokenKind::Keyword && token.lexeme != "true" && token.lexeme != "false") {
            error(token, "expected an expression");
            return nullptr;
        }
        auto result = std::make_unique<Expr>();
        result->kind = ExprKind::Literal;
        result->value = token.lexeme;
        result->span = token.span;
        return result;
    }

    if (match(TokenKind::Identifier)) {
        auto result = std::make_unique<Expr>();
        result->kind = ExprKind::Name;
        result->value = previous().lexeme;
        result->span = previous().span;
        return finish_postfix(std::move(result));
    }

    if (match(TokenKind::LeftBracket)) {
        auto result = std::make_unique<Expr>();
        result->kind = ExprKind::ListLiteral;
        const auto start = previous().span.start;
        if (!check(TokenKind::RightBracket)) {
            do {
                auto element = expression();
                if (!element) {
                    return nullptr;
                }
                result->arguments.push_back(std::move(element));
            } while (match(TokenKind::Comma));
        }
        const auto* closing = consume(TokenKind::RightBracket, "expected ']' after list elements");
        if (closing == nullptr) {
            return nullptr;
        }
        result->span = {start, closing->span.end};
        return finish_postfix(std::move(result));
    }

    if (match(TokenKind::LeftParen)) {
        auto result = expression();
        if (consume(TokenKind::RightParen, "expected ')' after expression") == nullptr) {
            return nullptr;
        }
        return result;
    }

    error(peek(), "expected an expression");
    return nullptr;
}

std::unique_ptr<Expr> Parser::finish_postfix(std::unique_ptr<Expr> expression) {
    while (true) {
        if (check(TokenKind::LeftParen) && expression->kind == ExprKind::Name) {
            advance();
            expression = finish_call(std::move(expression));
            if (!expression) {
                return nullptr;
            }
        } else if (match(TokenKind::LeftBracket)) {
            auto index = this->expression();
            if (!index) {
                return nullptr;
            }
            const auto* closing = consume(TokenKind::RightBracket, "expected ']' after index");
            if (closing == nullptr) {
                return nullptr;
            }
            auto indexed = std::make_unique<Expr>();
            indexed->kind = ExprKind::Index;
            indexed->span = {expression->span.start, closing->span.end};
            indexed->left = std::move(expression);
            indexed->right = std::move(index);
            expression = std::move(indexed);
        } else {
            return expression;
        }
    }
}

std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee) {
    auto result = std::make_unique<Expr>();
    result->kind = ExprKind::Call;
    result->span.start = callee->span.start;
    result->value = callee->value;
    if (!check(TokenKind::RightParen)) {
        do {
            auto argument = expression();
            if (!argument) {
                return nullptr;
            }
            result->arguments.push_back(std::move(argument));
        } while (match(TokenKind::Comma));
    }
    const auto* closing = consume(TokenKind::RightParen, "expected ')' after arguments");
    if (closing == nullptr) {
        return nullptr;
    }
    result->span.end = closing->span.end;
    return result;
}

int Parser::precedence(TokenKind kind) {
    switch (kind) {
    case TokenKind::OrOr: return 1;
    case TokenKind::AndAnd: return 2;
    case TokenKind::EqualEqual:
    case TokenKind::BangEqual: return 3;
    case TokenKind::Less:
    case TokenKind::LessEqual:
    case TokenKind::Greater:
    case TokenKind::GreaterEqual: return 4;
    case TokenKind::Plus:
    case TokenKind::Minus: return 5;
    case TokenKind::Star:
    case TokenKind::Slash:
    case TokenKind::Percent: return 6;
    default: return 0;
    }
}

}