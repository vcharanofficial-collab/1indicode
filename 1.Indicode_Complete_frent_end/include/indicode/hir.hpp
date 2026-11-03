#pragma once

#include "indicode/ast.hpp"
#include "indicode/diagnostic.hpp"

#include <memory>
#include <string>
#include <vector>

namespace indicode {

enum class HirExprKind {
    Name,
    Literal,
    Unary,
    Binary,
    Call,
    ListLiteral,
    Index
};

struct HirExpr {
    HirExprKind kind;
    std::string type;
    SourceSpan span;
    std::string value;
    std::string symbol;
    std::unique_ptr<HirExpr> left;
    std::unique_ptr<HirExpr> right;
    std::vector<std::unique_ptr<HirExpr>> arguments;
    std::vector<std::string> argument_names;
};

enum class HirStmtKind {
    Variable,
    Assignment,
    Return,
    Expression,
    If,
    While,
    Repeat,
    ForEach
};

struct HirStmt {
    HirStmtKind kind;
    SourceSpan span;
    std::string name;
    std::string type;
    bool mutable_binding = true;
    std::unique_ptr<HirExpr> expression;
    std::vector<std::unique_ptr<HirStmt>> body;
    std::vector<std::unique_ptr<HirStmt>> else_body;
};

struct HirParameter {
    std::string name;
    std::string type;
    SourceSpan span;
};

struct HirFunction {
    std::string name;
    std::vector<HirParameter> parameters;
    std::string return_type;
    SourceSpan span;
    std::vector<std::unique_ptr<HirStmt>> body;
};

struct HirProgram {
    std::vector<HirFunction> functions;
    std::vector<std::unique_ptr<HirStmt>> statements;
};

HirProgram lower_to_hir(const Program& program);
std::vector<Diagnostic> verify_hir(const HirProgram& program);
std::string print_hir(const HirProgram& program);

}