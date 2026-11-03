#include "indicode/hir.hpp"

#include <functional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace indicode {
namespace {

std::unique_ptr<HirExpr> lower_expression(const Expr& expression) {
    auto result = std::make_unique<HirExpr>();
    result->kind = static_cast<HirExprKind>(expression.kind);
    result->type = expression.inferred_type;
    result->span = expression.span;
    result->value = expression.value;
    result->symbol = expression.resolved_symbol;
    if (expression.left) {
        result->left = lower_expression(*expression.left);
    }
    if (expression.right) {
        result->right = lower_expression(*expression.right);
    }
    for (const auto& argument : expression.arguments) {
        result->arguments.push_back(lower_expression(*argument));
    }
    if (result->kind == HirExprKind::Call && result->symbol.empty()) {
        result->symbol = result->value;
    }
    return result;
}

std::unique_ptr<HirStmt> lower_statement(const Stmt& statement) {
    auto result = std::make_unique<HirStmt>();
    result->kind = static_cast<HirStmtKind>(statement.kind);
    result->span = statement.span;
    result->name = statement.name;
    result->type = statement.declared_type;
    result->mutable_binding = statement.mutable_binding;
    if (statement.expression) {
        result->expression = lower_expression(*statement.expression);
    }
    for (const auto& child : statement.body) {
        result->body.push_back(lower_statement(*child));
    }
    for (const auto& child : statement.else_body) {
        result->else_body.push_back(lower_statement(*child));
    }
    return result;
}

void lower_statements(const std::vector<std::unique_ptr<Stmt>>& statements,
                      std::vector<std::unique_ptr<HirStmt>>& output) {
    for (const auto& statement : statements) {
        output.push_back(lower_statement(*statement));
    }
}

bool is_integer_type(const std::string& type) {
    return type == "Byte" || type == "U8" || type == "Int2" || type == "UInt2" ||
           type == "Int" || type == "UInt" || type == "Long" || type == "ULong";
}

bool is_numeric_type(const std::string& type) {
    return is_integer_type(type) || type == "Half" || type == "Float" || type == "Double";
}

bool is_known_type(const std::string& type) {
    constexpr std::string_view list_prefix = "List<";
    if (type.starts_with(list_prefix) && type.size() > list_prefix.size() && type.back() == '>') {
        const auto element_type = type.substr(list_prefix.size(), type.size() - list_prefix.size() - 1);
        return is_known_type(element_type) && element_type != "Void";
    }
    static const std::unordered_set<std::string> types{
        "Bool", "Byte", "U8", "Int2", "UInt2", "Int", "UInt", "Long", "ULong",
        "Half", "Float", "Double", "Char", "Text", "Void"
    };
    return types.contains(type);
}

std::string list_element_type(const std::string& type) {
    constexpr std::string_view list_prefix = "List<";
    if (type.starts_with(list_prefix) && type.size() > list_prefix.size() && type.back() == '>') {
        return type.substr(list_prefix.size(), type.size() - list_prefix.size() - 1);
    }
    return {};
}

}

HirProgram lower_to_hir(const Program& program) {
    HirProgram result;
    for (const auto& function : program.functions) {
        HirFunction lowered;
        lowered.name = function.name;
        lowered.return_type = function.return_type.empty() ? "Void" : function.return_type;
        lowered.span = function.span;
        for (const auto& parameter : function.parameters) {
            lowered.parameters.push_back({parameter.name, parameter.type, parameter.span});
        }
        lower_statements(function.body, lowered.body);
        result.functions.push_back(std::move(lowered));
    }
    lower_statements(program.statements, result.statements);
    return result;
}

std::vector<Diagnostic> verify_hir(const HirProgram& program) {
    std::vector<Diagnostic> diagnostics;
    std::unordered_map<std::string, const HirFunction*> functions;
    for (const auto& function : program.functions) {
        if (!functions.emplace(function.name, &function).second) {
            diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-001",
                                   "duplicate function in typed HIR", function.span});
        }
        if (!is_known_type(function.return_type)) {
            diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-002",
                                   "function has an unknown return type", function.span});
        }
        for (const auto& parameter : function.parameters) {
            if (!is_known_type(parameter.type) || parameter.type == "Void") {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-003",
                                       "parameter has an invalid type", parameter.span});
            }
        }
    }

    std::function<std::string(const HirExpr&)> verify_expression;
    verify_expression = [&](const HirExpr& expression) -> std::string {
        if (!is_known_type(expression.type)) {
            diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-004",
                                   "expression is missing a valid type annotation", expression.span});
            return "<error>";
        }

        auto child_type = [&](const std::unique_ptr<HirExpr>& child) {
            if (!child) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-005",
                                       "expression node is missing a required child", expression.span});
                return std::string("<error>");
            }
            return verify_expression(*child);
        };

        switch (expression.kind) {
        case HirExprKind::Name:
            if (expression.symbol.empty()) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-006",
                                       "name expression has no resolved symbol", expression.span});
            }
            break;
        case HirExprKind::Literal:
            if (expression.type == "Void") {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-007",
                                       "literal cannot have type Void", expression.span});
            }
            break;
        case HirExprKind::Unary: {
            const auto operand_type = child_type(expression.left);
            const bool valid = (expression.value == "!" && operand_type == "Bool" && expression.type == "Bool") ||
                               ((expression.value == "+" || expression.value == "-") &&
                                is_numeric_type(operand_type) && expression.type == operand_type) ||
                               (expression.value == "~" && is_integer_type(operand_type) &&
                                expression.type == operand_type);
            if (!valid && operand_type != "<error>") {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-008",
                                       "unary operation has inconsistent operand/result types", expression.span});
            }
            break;
        }
        case HirExprKind::Binary: {
            const auto left_type = child_type(expression.left);
            const auto right_type = child_type(expression.right);
            if (left_type == "<error>" || right_type == "<error>") {
                break;
            }
            const bool arithmetic = expression.value == "+" || expression.value == "-" ||
                                    expression.value == "*" || expression.value == "/" || expression.value == "%";
            const bool logical = expression.value == "&&" || expression.value == "||";
            const bool equality = expression.value == "==" || expression.value == "!=";
            const bool ordering = expression.value == "<" || expression.value == "<=" ||
                                  expression.value == ">" || expression.value == ">=";
            const bool valid = arithmetic
                ? left_type == right_type && is_numeric_type(left_type) && expression.type == left_type
                : logical
                    ? left_type == "Bool" && right_type == "Bool" && expression.type == "Bool"
                    : equality
                        ? left_type == right_type && expression.type == "Bool"
                        : ordering
                            ? left_type == right_type &&
                              (is_numeric_type(left_type) || left_type == "Char" || left_type == "Text") &&
                              expression.type == "Bool"
                            : false;
            if (!valid) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-009",
                                       "binary operation has inconsistent operand/result types", expression.span});
            }
            break;
        }
        case HirExprKind::Call: {
            std::vector<std::string> argument_types;
            for (const auto& argument : expression.arguments) {
                argument_types.push_back(verify_expression(*argument));
            }
            if (expression.symbol == "say") {
                if (argument_types.size() != 1 || expression.type != "Void") {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-010",
                                           "say call has an invalid signature", expression.span});
                }
                break;
            }
            const auto target = functions.find(expression.symbol);
            if (target == functions.end()) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-011",
                                       "call target is not present in the typed HIR", expression.span});
                break;
            }
            const auto& parameters = target->second->parameters;
            bool matches = parameters.size() == argument_types.size() &&
                           expression.type == target->second->return_type;
            for (std::size_t index = 0; matches && index < parameters.size(); ++index) {
                matches = argument_types[index] == parameters[index].type;
            }
            if (!matches) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-012",
                                       "call signature does not match its typed HIR target", expression.span});
            }
            break;
        }
        case HirExprKind::ListLiteral: {
            if (expression.arguments.empty()) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-019",
                                       "typed list must have at least one element", expression.span});
                break;
            }
            const auto element_type = verify_expression(*expression.arguments.front());
            bool matches = !element_type.empty() && element_type != "Void" &&
                           expression.type == "List<" + element_type + ">";
            for (std::size_t index = 1; index < expression.arguments.size(); ++index) {
                matches = matches && verify_expression(*expression.arguments[index]) == element_type;
            }
            if (!matches) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-020",
                                       "list elements do not match the list's declared element type", expression.span});
            }
            break;
        }
        case HirExprKind::Index: {
            const auto collection_type = child_type(expression.left);
            const auto index_type = child_type(expression.right);
            const auto element_type = list_element_type(collection_type);
            const bool valid_list = !element_type.empty() && expression.type == element_type;
            const bool valid_text = collection_type == "Text" && expression.type == "Char";
            if (index_type != "Int" || (!valid_list && !valid_text)) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-021",
                                       "index operation has inconsistent collection, index, or result types",
                                       expression.span});
            }
            break;
        }
        }
        return expression.type;
    };

    std::function<void(const std::vector<std::unique_ptr<HirStmt>>&, const std::string&)> verify_statements;
    verify_statements = [&](const std::vector<std::unique_ptr<HirStmt>>& statements,
                            const std::string& return_type) {
        for (const auto& statement : statements) {
            const bool needs_expression = statement->kind != HirStmtKind::Return || statement->expression != nullptr;
            std::string value_type;
            if (needs_expression && statement->expression) {
                value_type = verify_expression(*statement->expression);
            } else if (needs_expression && statement->kind != HirStmtKind::Return) {
                diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-013",
                                       "statement is missing its expression", statement->span});
            }

            switch (statement->kind) {
            case HirStmtKind::Variable:
                if (!is_known_type(statement->type) || value_type != statement->type) {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-014",
                                           "variable initializer does not match its declared HIR type", statement->span});
                }
                break;
            case HirStmtKind::Assignment:
                if (statement->name.empty() || !is_known_type(statement->type) || value_type != statement->type) {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-015",
                                           "assignment does not match its resolved HIR binding type", statement->span});
                }
                break;
            case HirStmtKind::Return:
                if ((statement->expression ? value_type : "Void") != return_type) {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-016",
                                           "return type does not match the containing function", statement->span});
                }
                break;
            case HirStmtKind::Expression:
                break;
            case HirStmtKind::If:
            case HirStmtKind::While:
                if (value_type != "Bool") {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-017",
                                           "condition must have type Bool", statement->span});
                }
                break;
            case HirStmtKind::Repeat:
                if (!is_integer_type(value_type)) {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-018",
                                           "repeat count must have an integer type", statement->span});
                }
                break;
            case HirStmtKind::ForEach: {
                const auto element_type = value_type == "Text" ? "Char" : list_element_type(value_type);
                if (statement->name.empty() || element_type.empty() || statement->type != element_type) {
                    diagnostics.push_back({DiagnosticSeverity::Error, "E-HIR-022",
                                           "for-each binding does not match its iterable element type",
                                           statement->span});
                }
                break;
            }
            }
            verify_statements(statement->body, return_type);
            verify_statements(statement->else_body, return_type);
        }
    };

    for (const auto& function : program.functions) {
        verify_statements(function.body, function.return_type);
    }
    verify_statements(program.statements, "Void");
    return diagnostics;
}

std::string print_hir(const HirProgram& program) {
    std::ostringstream output;
    std::function<void(const HirExpr&)> print_expression;
    print_expression = [&](const HirExpr& expression) {
        switch (expression.kind) {
        case HirExprKind::Name:
            output << '%' << expression.symbol << ':' << expression.type;
            break;
        case HirExprKind::Literal:
            output << expression.value << ':' << expression.type;
            break;
        case HirExprKind::Unary:
            output << '(' << expression.value << ' ';
            print_expression(*expression.left);
            output << ") : " << expression.type;
            break;
        case HirExprKind::Binary:
            output << '(' << expression.value << ' ';
            print_expression(*expression.left);
            output << ' ';
            print_expression(*expression.right);
            output << ") : " << expression.type;
            break;
        case HirExprKind::Call:
            output << "call @" << expression.symbol << '(';
            for (std::size_t index = 0; index < expression.arguments.size(); ++index) {
                if (index != 0) output << ", ";
                print_expression(*expression.arguments[index]);
            }
            output << ") : " << expression.type;
            break;
        case HirExprKind::ListLiteral:
            output << "list<" << expression.type << ">[";
            for (std::size_t index = 0; index < expression.arguments.size(); ++index) {
                if (index != 0) output << ", ";
                print_expression(*expression.arguments[index]);
            }
            output << ']';
            break;
        case HirExprKind::Index:
            output << "index ";
            print_expression(*expression.left);
            output << '[';
            print_expression(*expression.right);
            output << "] : " << expression.type;
            break;
        }
    };

    std::function<void(const std::vector<std::unique_ptr<HirStmt>>&, std::size_t)> print_statements;
    print_statements = [&](const std::vector<std::unique_ptr<HirStmt>>& statements, std::size_t depth) {
        for (const auto& statement : statements) {
            for (std::size_t index = 0; index < depth; ++index) output << "  ";
            switch (statement->kind) {
            case HirStmtKind::Variable:
                output << (statement->mutable_binding ? "var " : "let ") << '%' << statement->name
                       << ": " << statement->type << " = ";
                print_expression(*statement->expression);
                output << '\n';
                break;
            case HirStmtKind::Assignment:
                output << '%' << statement->name << ": " << statement->type << " = ";
                print_expression(*statement->expression);
                output << '\n';
                break;
            case HirStmtKind::Return:
                output << "return";
                if (statement->expression) {
                    output << ' ';
                    print_expression(*statement->expression);
                }
                output << '\n';
                break;
            case HirStmtKind::Expression:
                print_expression(*statement->expression);
                output << '\n';
                break;
            case HirStmtKind::If:
                output << "if "; print_expression(*statement->expression); output << '\n';
                print_statements(statement->body, depth + 1);
                if (!statement->else_body.empty()) {
                    for (std::size_t index = 0; index < depth; ++index) output << "  ";
                    output << "else\n";
                    print_statements(statement->else_body, depth + 1);
                }
                break;
            case HirStmtKind::While:
                output << "while "; print_expression(*statement->expression); output << '\n';
                print_statements(statement->body, depth + 1);
                break;
            case HirStmtKind::Repeat:
                output << "repeat "; print_expression(*statement->expression); output << " times\n";
                print_statements(statement->body, depth + 1);
                break;
            case HirStmtKind::ForEach:
                output << "for %" << statement->name << ": " << statement->type << " in ";
                print_expression(*statement->expression);
                output << '\n';
                print_statements(statement->body, depth + 1);
                break;
            }
        }
    };

    for (const auto& function : program.functions) {
        output << "func @" << function.name << '(';
        for (std::size_t index = 0; index < function.parameters.size(); ++index) {
            if (index != 0) output << ", ";
            output << '%' << function.parameters[index].name << ": " << function.parameters[index].type;
        }
        output << ") -> " << function.return_type << '\n';
        print_statements(function.body, 1);
    }
    print_statements(program.statements, 0);
    return output.str();
}

}