#pragma once

#include "indicode/source.hpp"

#include <memory>
#include <string>
#include <vector>

namespace indicode {

enum class ExprKind {
    Name,
    Literal,
    Unary,
    Binary,
    Call,
    ListLiteral,
    Index
};

struct Expr {
    ExprKind kind;
    SourceSpan span;
    std::string value;
    std::string inferred_type;
    std::string resolved_symbol;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    std::vector<std::unique_ptr<Expr>> arguments;
};

enum class StmtKind {
    Variable,
    Assignment,
    Return,
    Expression,
    If,
    While,
    Repeat,
    ForEach
};

struct Stmt {
    StmtKind kind;
    SourceSpan span;
    std::string name;
    std::string declared_type;
    bool mutable_binding = true;
    std::unique_ptr<Expr> expression;
    std::vector<std::unique_ptr<Stmt>> body;
    std::vector<std::unique_ptr<Stmt>> else_body;
};

struct Parameter {
    std::string name;
    std::string type;
    SourceSpan span;
};

struct FunctionDecl {
    std::string name;
    std::vector<Parameter> parameters;
    std::string return_type;
    SourceSpan span;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct Program {
    std::vector<FunctionDecl> functions;
    std::vector<std::unique_ptr<Stmt>> statements;
};

std::string print_program(const Program& program, bool include_types = false);

}