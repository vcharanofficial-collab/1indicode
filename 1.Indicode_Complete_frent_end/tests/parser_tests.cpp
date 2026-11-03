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
        "for index in 0..10 { say(index) }\n"
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
        require(parsed.program.statements.size() == 8,
            "calls, named arguments, repeat/for-each loops, lists, indexing, and ranges should parse");
    require(parsed.program.statements[2]->expression->kind == indicode::ExprKind::ListLiteral,
            "bracketed values should parse as list literals");
    require(parsed.program.statements[3]->expression->kind == indicode::ExprKind::Index,
            "postfix brackets should parse as indexing");
    require(parsed.program.statements[4]->kind == indicode::StmtKind::ForEach &&
                parsed.program.statements[4]->name == "item",
            "for-each should retain its iteration binding");
    require(parsed.program.statements[7]->kind == indicode::StmtKind::ForEach &&
                parsed.program.statements[7]->expression->kind == indicode::ExprKind::Range,
            "integer ranges should parse as for-each iterables");
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

    const indicode::SourceFile recoverable{
        "recoverable.indi",
        "function broken(value Int) { return value }\n"
        "survivor = 42\n"
    };
    const auto recoverable_lexed = indicode::Lexer(recoverable).lex();
    const auto recoverable_result = indicode::Parser(recoverable_lexed.tokens).parse();
    require(!recoverable_result.diagnostics.empty() &&
                recoverable_result.program.statements.size() == 1 &&
                recoverable_result.program.statements[0]->name == "survivor",
            "parser recovery should retain valid declarations after a malformed function");

    const std::vector<indicode::Token> empty_tokens;
    const auto empty_result = indicode::Parser(empty_tokens).parse();
    require(empty_result.diagnostics.empty() && empty_result.program.statements.empty(),
            "empty token input should produce an empty program without crashing");

    const indicode::SourceFile records_source{
        "records.indi",
        "record Point { x: Int, y: Int }\n"
        "origin = Point(x: 0, y: 0)\n"
        "horizontal = origin.x\n"
    };
    const auto records_lexed = indicode::Lexer(records_source).lex();
    const auto records_parsed = indicode::Parser(records_lexed.tokens).parse();
    require(records_lexed.diagnostics.empty() && records_parsed.diagnostics.empty(),
            "record declarations, named construction, and member access should parse");
    require(records_parsed.program.records.size() == 1 &&
                records_parsed.program.records[0].fields.size() == 2 &&
                records_parsed.program.statements[1]->expression->kind == indicode::ExprKind::Member,
            "record fields and member access should be represented in the AST");

    const indicode::SourceFile tensor_source{
        "tensor.indi",
        "function identity(input: Tensor<Float, [Batch, 3]>) -> Tensor<Float, [Batch, 3]> "
        "{ return input }\n"
    };
    const auto tensor_lexed = indicode::Lexer(tensor_source).lex();
    const auto tensor_parsed = indicode::Parser(tensor_lexed.tokens).parse();
    require(tensor_lexed.diagnostics.empty() && tensor_parsed.diagnostics.empty(),
            "tensor element types and symbolic/static dimensions should parse");
    require(tensor_parsed.program.functions[0].parameters[0].type == "Tensor<Float,[Batch,3]>" &&
                tensor_parsed.program.functions[0].return_type == "Tensor<Float,[Batch,3]>",
            "tensor dimensions should be preserved canonically in AST types");

    const indicode::SourceFile range_source{"range.indi", "value = 1..5 + 2\n"};
    const auto range_lexed = indicode::Lexer(range_source).lex();
    const auto range_parsed = indicode::Parser(range_lexed.tokens).parse();
    require(range_lexed.diagnostics.empty() && range_parsed.diagnostics.empty() &&
                range_parsed.program.statements[0]->expression->kind == indicode::ExprKind::Range &&
                range_parsed.program.statements[0]->expression->right->kind == indicode::ExprKind::Binary,
            "range parsing should preserve endpoint arithmetic precedence");

    const indicode::SourceFile else_if_source{
        "else_if.indi",
        "function classify(value: Int) -> Int {\n"
        "  if value == 0 { return 0 } else if value == 1 { return 1 } else { return 2 }\n"
        "}\n"
    };
    const auto else_if_lexed = indicode::Lexer(else_if_source).lex();
    const auto else_if_parsed = indicode::Parser(else_if_lexed.tokens).parse();
    require(else_if_lexed.diagnostics.empty() && else_if_parsed.diagnostics.empty() &&
                else_if_parsed.program.functions[0].body[0]->else_body.size() == 1 &&
                else_if_parsed.program.functions[0].body[0]->else_body[0]->kind == indicode::StmtKind::If,
            "else-if chains should parse as nested conditional statements");

    std::cout << "Parser tests passed\n";
}