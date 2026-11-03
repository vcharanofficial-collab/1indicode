#include "indicode/sema.hpp"

#include <charconv>
#include <cstdint>
#include <limits>
#include <utility>

namespace indicode {
namespace {

constexpr const char* error_type = "<error>";

bool is_integer_type(const std::string& type) {
    return type == "Byte" || type == "U8" || type == "Int2" || type == "UInt2" ||
           type == "Int" || type == "UInt" || type == "Long" || type == "ULong";
}

bool is_numeric_type(const std::string& type) {
    return is_integer_type(type) || type == "Half" || type == "Float" || type == "Double";
}

std::string get_list_element_type(const std::string& type) {
    constexpr std::string_view prefix = "List<";
    if (type.starts_with(prefix) && type.size() > prefix.size() && type.back() == '>') {
        return type.substr(prefix.size(), type.size() - prefix.size() - 1);
    }
    return {};
}

bool parse_integer_magnitude(const std::string& spelling, std::uint64_t& value) {
    std::string digits;
    digits.reserve(spelling.size());
    for (const char character : spelling) {
        if (character != '_') digits.push_back(character);
    }
    unsigned base = 10;
    std::size_t prefix_length = 0;
    if (digits.size() >= 2 && digits[0] == '0') {
        if (digits[1] == 'b' || digits[1] == 'B') { base = 2; prefix_length = 2; }
        else if (digits[1] == 'o' || digits[1] == 'O') { base = 8; prefix_length = 2; }
        else if (digits[1] == 'x' || digits[1] == 'X') { base = 16; prefix_length = 2; }
    }
    if (prefix_length == digits.size()) return false;
    const auto* begin = digits.data() + prefix_length;
    const auto* end = digits.data() + digits.size();
    const auto parsed = std::from_chars(begin, end, value, static_cast<int>(base));
    return parsed.ec == std::errc{} && parsed.ptr == end;
}

bool is_integer_spelling(const std::string& spelling) {
    return spelling.size() < 2 || spelling[0] != '0' ||
           (spelling[1] != 'b' && spelling[1] != 'B' && spelling[1] != 'o' &&
            spelling[1] != 'O' && spelling[1] != 'x' && spelling[1] != 'X');
}

bool is_integer_literal_expression(const Expr& expression) {
    if (expression.kind == ExprKind::Literal) {
        return !expression.value.empty() && expression.value.front() >= '0' &&
               expression.value.front() <= '9';
    }
    return expression.kind == ExprKind::Unary && expression.value == "-" && expression.left &&
           expression.left->kind == ExprKind::Literal && !expression.left->value.empty() &&
           expression.left->value.front() >= '0' && expression.left->value.front() <= '9';
}

std::string literal_string_type(const std::string& spelling) {
    if (spelling.size() < 2) return "Text";
    std::size_t scalar_count = 0;
    for (std::size_t index = 1; index + 1 < spelling.size(); ++index) {
        const auto value = static_cast<unsigned char>(spelling[index]);
        if (value == '\\') {
            ++scalar_count;
            ++index;
            if (index + 1 < spelling.size() && spelling[index] == 'u' && spelling[index + 1] == '{') {
                index += 2;
                while (index + 1 < spelling.size() && spelling[index] != '}') ++index;
            }
        } else {
            ++scalar_count;
            if ((value & 0xe0) == 0xc0) index += 1;
            else if ((value & 0xf0) == 0xe0) index += 2;
            else if ((value & 0xf8) == 0xf0) index += 3;
        }
    }
    return scalar_count == 1 ? "Char" : "Text";
}

}

SemanticResult Analyzer::analyze(Program& program) {
    diagnostics_.clear();
    functions_.clear();
    scopes_.clear();
    push_scope();

    for (const auto& function : program.functions) {
        if (function.name == "say" || functions_.contains(function.name)) {
            add_error("E-SEMA-001", "function name is already declared", function.span);
            continue;
        }
        FunctionSignature signature;
        signature.return_type = function.return_type.empty() ? "Void" : function.return_type;
        for (const auto& parameter : function.parameters) {
            signature.parameter_names.push_back(parameter.name);
            signature.parameter_types.push_back(parameter.type);
            if (!is_known_type(parameter.type)) {
                add_error("E-SEMA-002", "unknown parameter type", parameter.span);
            }
        }
        if (!is_known_type(signature.return_type)) {
            add_error("E-SEMA-003", "unknown function return type", function.span);
        }
        functions_.emplace(function.name, std::move(signature));
    }

    analyze_statements(program.statements);
    for (auto& function : program.functions) {
        push_scope();
        in_function_ = true;
        current_return_type_ = function.return_type.empty() ? "Void" : function.return_type;
        for (const auto& parameter : function.parameters) {
            declare_binding(parameter.name, {parameter.type, false, parameter.span}, parameter.span);
        }
        analyze_statements(function.body);
        if (current_return_type_ != "Void" && !always_returns(function.body)) {
            add_error("E-SEMA-011", "not all function paths return a value", function.span);
        }
        in_function_ = false;
        current_return_type_ = "Void";
        pop_scope();
    }

    pop_scope();
    return {std::move(diagnostics_)};
}

void Analyzer::add_error(const char* code, const char* message, SourceSpan span) {
    diagnostics_.push_back({DiagnosticSeverity::Error, code, message, span});
}

bool Analyzer::is_known_type(const std::string& type) const {
    if (const auto element_type = list_element_type(type); !element_type.empty()) {
        return is_known_type(element_type) && element_type != "Void";
    }
    return type == "Bool" || type == "Byte" || type == "U8" || type == "Int2" ||
           type == "UInt2" || type == "Int" || type == "UInt" || type == "Long" ||
           type == "ULong" || type == "Half" || type == "Float" || type == "Double" ||
           type == "Char" || type == "Text" || type == "Void";
}

std::string Analyzer::list_element_type(const std::string& type) const {
    return get_list_element_type(type);
}

void Analyzer::push_scope() {
    scopes_.emplace_back();
}

void Analyzer::pop_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

Analyzer::Binding* Analyzer::find_binding(const std::string& name) {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        const auto found = scope->find(name);
        if (found != scope->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

void Analyzer::declare_binding(const std::string& name, Binding binding, SourceSpan span) {
    if (scopes_.back().contains(name)) {
        add_error("E-SEMA-004", "name is already declared in this scope", span);
        return;
    }
    scopes_.back().emplace(name, std::move(binding));
}

void Analyzer::analyze_statements(std::vector<std::unique_ptr<Stmt>>& statements) {
    for (auto& statement : statements) {
        analyze_statement(*statement);
    }
}

void Analyzer::analyze_statement(Stmt& statement) {
    switch (statement.kind) {
    case StmtKind::Variable: {
            const bool has_explicit_type = !statement.declared_type.empty();
            const auto type = statement.declared_type;
        std::string value_type;
            if (has_explicit_type && !list_element_type(type).empty() &&
                statement.expression->kind == ExprKind::ListLiteral &&
                statement.expression->arguments.empty()) {
                statement.expression->inferred_type = type;
                value_type = type;
            } else if (has_explicit_type && is_integer_type(type) &&
            is_integer_literal_expression(*statement.expression)) {
            if (!type_integer_literal(*statement.expression, type)) {
                add_error("E-SEMA-022", "integer literal is outside the declared type range",
                          statement.expression->span);
                statement.expression->inferred_type = type;
            }
            value_type = type;
        } else {
            value_type = analyze_expression(*statement.expression);
        }
            if (has_explicit_type && !is_known_type(type)) {
            add_error("E-SEMA-002", "unknown variable type", statement.span);
            } else if (has_explicit_type && type != value_type && value_type != error_type) {
            add_error("E-SEMA-005", "initializer type does not match declared type", statement.span);
        }
            const auto resolved_type = has_explicit_type ? type : value_type;
            statement.declared_type = resolved_type;
            declare_binding(statement.name, {resolved_type, statement.mutable_binding, statement.span}, statement.span);
        break;
    }
    case StmtKind::Assignment: {
        const auto value_type = analyze_expression(*statement.expression);
        auto* binding = find_binding(statement.name);
        if (binding == nullptr) {
            statement.declared_type = value_type;
            declare_binding(statement.name, {value_type, true, statement.span}, statement.span);
        } else {
            statement.declared_type = binding->type;
            if (!binding->mutable_binding) {
                add_error("E-SEMA-006", "cannot assign to an immutable binding", statement.span);
            } else if (binding->type != value_type && value_type != error_type) {
                add_error("E-SEMA-005", "assignment type does not match variable type", statement.span);
            }
        }
        break;
    }
    case StmtKind::Return: {
        if (!in_function_) {
            add_error("E-SEMA-007", "return is only valid inside a function", statement.span);
            break;
        }
        const auto value_type = statement.expression ? analyze_expression(*statement.expression) : "Void";
        if (value_type != current_return_type_ && value_type != error_type) {
            add_error("E-SEMA-008", "return value does not match function return type", statement.span);
        }
        break;
    }
    case StmtKind::Expression:
        analyze_expression(*statement.expression);
        break;
    case StmtKind::If: {
        const auto condition_type = analyze_expression(*statement.expression);
        if (condition_type != "Bool" && condition_type != error_type) {
            add_error("E-SEMA-009", "if condition must have type Bool", statement.expression->span);
        }
        push_scope();
        analyze_statements(statement.body);
        pop_scope();
        if (!statement.else_body.empty()) {
            push_scope();
            analyze_statements(statement.else_body);
            pop_scope();
        }
        break;
    }
    case StmtKind::While: {
        const auto condition_type = analyze_expression(*statement.expression);
        if (condition_type != "Bool" && condition_type != error_type) {
            add_error("E-SEMA-009", "while condition must have type Bool", statement.expression->span);
        }
        push_scope();
        analyze_statements(statement.body);
        pop_scope();
        break;
    }
    case StmtKind::Repeat: {
        const auto count_type = analyze_expression(*statement.expression);
        if (!is_integer_type(count_type) && count_type != error_type) {
            add_error("E-SEMA-010", "repeat count must be an integer", statement.expression->span);
        }
        push_scope();
        analyze_statements(statement.body);
        pop_scope();
        break;
    }
    case StmtKind::ForEach: {
        const auto iterable_type = analyze_expression(*statement.expression);
        const auto element_type = iterable_type == "Text" ? "Char" : list_element_type(iterable_type);
        if (element_type.empty()) {
            if (iterable_type != error_type) {
                add_error("E-SEMA-023", "for-each requires a List<T> or Text value", statement.expression->span);
            }
            break;
        }
        statement.declared_type = element_type;
        push_scope();
        declare_binding(statement.name, {element_type, false, statement.span}, statement.span);
        analyze_statements(statement.body);
        pop_scope();
        break;
    }
    }
}

std::string Analyzer::analyze_expression(Expr& expression) {
    std::string type = error_type;
    switch (expression.kind) {
    case ExprKind::Literal:
        if (expression.value == "true" || expression.value == "false") {
            type = "Bool";
        } else if (!expression.value.empty() && expression.value.front() == '"') {
            type = literal_string_type(expression.value);
        } else if (is_integer_spelling(expression.value) &&
                   expression.value.find_first_of(".eE") != std::string::npos) {
            type = "Float";
        } else {
            std::uint64_t magnitude = 0;
            if (!parse_integer_magnitude(expression.value, magnitude)) {
                add_error("E-SEMA-012", "integer literal is outside the supported range", expression.span);
            } else if (magnitude > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
                add_error("E-SEMA-012", "integer literal is outside the Int range", expression.span);
            } else {
                type = "Int";
            }
        }
        break;
    case ExprKind::Name: {
        auto* binding = find_binding(expression.value);
        if (binding == nullptr) {
            add_error("E-SEMA-013", "unknown variable name", expression.span);
        } else {
            type = binding->type;
            expression.resolved_symbol = expression.value;
        }
        break;
    }
    case ExprKind::Unary: {
        const auto operand_type = analyze_expression(*expression.left);
        if (expression.value == "!" && operand_type == "Bool") {
            type = "Bool";
        } else if ((expression.value == "+" || expression.value == "-") && is_numeric_type(operand_type)) {
            type = operand_type;
        } else if (expression.value == "~" && is_integer_type(operand_type)) {
            type = operand_type;
        } else if (operand_type != error_type) {
            add_error("E-SEMA-014", "invalid operand type for unary operator", expression.span);
        }
        break;
    }
    case ExprKind::Binary: {
        const auto left_type = analyze_expression(*expression.left);
        const auto right_type = analyze_expression(*expression.right);
        if (expression.value == "&&" || expression.value == "||") {
            if (left_type == "Bool" && right_type == "Bool") {
                type = "Bool";
            } else if (left_type != error_type && right_type != error_type) {
                add_error("E-SEMA-015", "logical operators require Bool operands", expression.span);
            }
        } else if (expression.value == "==" || expression.value == "!=" ||
                   expression.value == "<" || expression.value == "<=" ||
                   expression.value == ">" || expression.value == ">=") {
            if (left_type == right_type && left_type != error_type &&
                (is_numeric_type(left_type) || left_type == "Char" || left_type == "Text" ||
                 expression.value == "==" || expression.value == "!=")) {
                type = "Bool";
            } else if (left_type != error_type && right_type != error_type) {
                add_error("E-SEMA-016", "comparison operands have incompatible types", expression.span);
            }
        } else {
            type = numeric_binary_type(expression, left_type, right_type);
        }
        break;
    }
    case ExprKind::Call: {
        std::vector<std::string> argument_types;
        for (auto& argument : expression.arguments) {
            argument_types.push_back(analyze_expression(*argument));
        }
        if (expression.value == "say") {
            if (argument_types.size() != 1 ||
                (!expression.argument_names.empty() && !expression.argument_names[0].empty())) {
                add_error("E-SEMA-017", "say expects exactly one argument", expression.span);
            } else {
                type = "Void";
            }
        } else {
            const auto function = functions_.find(expression.value);
            if (function == functions_.end()) {
                add_error("E-SEMA-018", "unknown function", expression.span);
            } else if (function->second.parameter_types.size() != argument_types.size()) {
                add_error("E-SEMA-019", "function argument count does not match signature", expression.span);
            } else {
                std::vector<bool> assigned(argument_types.size(), false);
                bool arguments_match = true;
                std::size_t next_positional = 0;
                for (std::size_t index = 0; index < argument_types.size(); ++index) {
                    std::size_t parameter_index = argument_types.size();
                    const std::string argument_name = index < expression.argument_names.size()
                        ? expression.argument_names[index]
                        : "";
                    if (argument_name.empty()) {
                        while (next_positional < assigned.size() && assigned[next_positional]) ++next_positional;
                        if (next_positional < assigned.size()) parameter_index = next_positional++;
                    } else {
                        for (std::size_t candidate = 0; candidate < function->second.parameter_names.size(); ++candidate) {
                            if (function->second.parameter_names[candidate] == argument_name) {
                                parameter_index = candidate;
                                break;
                            }
                        }
                        if (parameter_index == argument_types.size()) {
                            add_error("E-SEMA-028", "unknown named argument", expression.arguments[index]->span);
                            arguments_match = false;
                            continue;
                        }
                    }
                    if (parameter_index >= assigned.size()) {
                        add_error("E-SEMA-029", "too many positional arguments", expression.arguments[index]->span);
                        arguments_match = false;
                        continue;
                    }
                    if (assigned[parameter_index]) {
                        add_error("E-SEMA-030", "function parameter was supplied more than once",
                                  expression.arguments[index]->span);
                        arguments_match = false;
                        continue;
                    }
                    assigned[parameter_index] = true;
                    if (argument_types[index] != error_type &&
                        argument_types[index] != function->second.parameter_types[parameter_index]) {
                        arguments_match = false;
                    }
                }
                for (std::size_t index = 0; index < assigned.size(); ++index) {
                    if (!assigned[index]) {
                        add_error("E-SEMA-031", "missing required function argument", expression.span);
                        arguments_match = false;
                    }
                }
                if (!arguments_match) {
                    add_error("E-SEMA-020", "function argument type does not match signature", expression.span);
                } else {
                    type = function->second.return_type;
                    expression.resolved_symbol = expression.value;
                }
            }
        }
        break;
    }
    case ExprKind::ListLiteral: {
        if (expression.arguments.empty()) {
            add_error("E-SEMA-024", "an empty list needs an explicit element type", expression.span);
            break;
        }
        const auto element_type = analyze_expression(*expression.arguments.front());
        bool homogeneous = element_type != error_type && element_type != "Void";
        for (std::size_t index = 1; index < expression.arguments.size(); ++index) {
            const auto current_type = analyze_expression(*expression.arguments[index]);
            homogeneous = homogeneous && current_type == element_type;
        }
        if (!homogeneous) {
            add_error("E-SEMA-025", "all list elements must have the same non-Void type", expression.span);
        } else {
            type = "List<" + element_type + ">";
        }
        break;
    }
    case ExprKind::Index: {
        const auto collection_type = analyze_expression(*expression.left);
        const auto index_type = analyze_expression(*expression.right);
        if (index_type != "Int" && index_type != error_type) {
            add_error("E-SEMA-026", "list index must have type Int", expression.right->span);
        }
        const auto element_type = list_element_type(collection_type);
        if (!element_type.empty()) {
            type = element_type;
        } else if (collection_type == "Text") {
            type = "Char";
        } else if (collection_type != error_type) {
            add_error("E-SEMA-027", "indexing requires a List<T> or Text value", expression.left->span);
        }
        break;
    }
    }
    expression.inferred_type = type;
    return type;
}

bool Analyzer::type_integer_literal(Expr& expression, const std::string& type) {
    const bool negative = expression.kind == ExprKind::Unary;
    Expr& literal = negative ? *expression.left : expression;
    std::uint64_t magnitude = 0;
    if (!parse_integer_magnitude(literal.value, magnitude)) {
        return false;
    }

    bool is_signed = false;
    std::uint64_t positive_limit = 0;
    std::uint64_t negative_limit = 0;
    if (type == "Byte" || type == "U8") positive_limit = 255;
    else if (type == "Int2") { is_signed = true; positive_limit = 32767; negative_limit = 32768; }
    else if (type == "UInt2") positive_limit = 65535;
    else if (type == "Int") { is_signed = true; positive_limit = 2147483647; negative_limit = 2147483648ULL; }
    else if (type == "UInt") positive_limit = 4294967295ULL;
    else if (type == "Long") {
        is_signed = true;
        positive_limit = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
        negative_limit = positive_limit + 1;
    } else if (type == "ULong") {
        positive_limit = std::numeric_limits<std::uint64_t>::max();
    } else {
        return false;
    }

    const bool in_range = negative ? is_signed && magnitude <= negative_limit
                                   : magnitude <= positive_limit;
    if (in_range) {
        literal.inferred_type = type;
        expression.inferred_type = type;
    }
    return in_range;
}

std::string Analyzer::numeric_binary_type(Expr& expression, const std::string& left,
                                          const std::string& right) {
    if (left == right && is_numeric_type(left)) {
        return left;
    }
    if (left != error_type && right != error_type) {
        add_error("E-SEMA-021", "arithmetic operands must have the same numeric type", expression.span);
    }
    return error_type;
}

bool Analyzer::always_returns(const std::vector<std::unique_ptr<Stmt>>& statements) const {
    for (const auto& statement : statements) {
        if (statement->kind == StmtKind::Return) {
            return true;
        }
        if (statement->kind == StmtKind::If && !statement->else_body.empty() &&
            always_returns(statement->body) && always_returns(statement->else_body)) {
            return true;
        }
    }
    return false;
}

}