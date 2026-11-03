#include "indicode/lexer.hpp"
#include "indicode/parser.hpp"

#include <cstdlib>
#include <iostream>

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
        "parser.indi",
        "function add(a: Int, b: Int) -> Int {\n"
        "  total = a + 2 * b\n"
        "  if total >= 10 { return total } else { return 0 }\n"
        "}\n"
        "say(\"ready\")\n"
        "repeat 3 times { say(\"go\") }\n"
        "items = [1, 2, 3]\n"
        "first = items[0]\n"
        "for item in items { say(item) }\n"
        "named = add(b: 2, a: 1)\n"
        "List<Int> empty = []\n"
        "function keep(values: List<Int>) -> List<Int> { return values }\n"
    };
    const auto lexed = indicode::Lexer(source).lex();
    const auto parsed = indicode::Parser(lexed.tokens).parse();

    require(lexed.diagnostics.empty(), "parser fixture should lex cleanly");
    require(parsed.diagnostics.empty(), "valid core syntax should parse without errors");
        require(parsed.program.functions.size() == 2, "function declarations should be captured");
    require(parsed.program.functions[0].parameters.size() == 2,
            "function parameters and types should be captured");
    require(parsed.program.functions[0].body.size() == 2,
            "function body statements should be captured");
    const auto& assignment = parsed.program.functions[0].body[0];
    require(assignment->kind == indicode::StmtKind::Assignment,
            "identifier assignment should form an assignment statement");
    require(assignment->expression->kind == indicode::ExprKind::Binary &&
                assignment->expression->value == "+",
            "addition should be the root of the expression");
    require(assignment->expression->right->kind == indicode::ExprKind::Binary &&
                assignment->expression->right->value == "*",
            "multiplication should bind more tightly than addition");
        require(parsed.program.statements.size() == 7,
            "calls, named arguments, repeat/for-each loops, lists, and indexing should parse");
    require(parsed.program.statements[2]->expression->kind == indicode::ExprKind::ListLiteral,
            "bracketed values should parse as list literals");
    require(parsed.program.statements[3]->expression->kind == indicode::ExprKind::Index,
            "postfix brackets should parse as indexing");
    require(parsed.program.statements[4]->kind == indicode::StmtKind::ForEach &&
                parsed.program.statements[4]->name == "item",
            "for-each should retain its iteration binding");
        require(parsed.program.statements[5]->expression->argument_names[0] == "b" &&
                                parsed.program.statements[5]->expression->argument_names[1] == "a",
                        "parser should retain named argument labels and order");
        require(parsed.program.statements[6]->declared_type == "List<Int>" &&
                                parsed.program.functions[1].parameters[0].type == "List<Int>" &&
                                parsed.program.functions[1].return_type == "List<Int>",
                        "generic list types should parse in declarations and function signatures");

        const indicode::SourceFile malformed{
                "bad.indi",
                "function broken(a Int) {\n  x = 1 +\n}\n"
                "call = add(a: 1, 2)\n"
        };
    const auto malformed_tokens = indicode::Lexer(malformed).lex();
    const auto malformed_result = indicode::Parser(malformed_tokens.tokens).parse();
    require(malformed_result.diagnostics.size() >= 2,
            "malformed syntax and positional-after-named calls should be diagnosed");
    require(malformed_result.diagnostics[0].code == "E-PARSE-001",
            "parser diagnostic code should be stable");

    const std::vector<indicode::Token> empty_tokens;
    const auto empty_result = indicode::Parser(empty_tokens).parse();
    require(empty_result.diagnostics.empty() && empty_result.program.statements.empty(),
            "empty token input should produce an empty program without crashing");

    std::cout << "Parser tests passed\n";
}