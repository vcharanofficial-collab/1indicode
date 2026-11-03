#include "indicode/hir.hpp"
#include "indicode/lexer.hpp"
#include "indicode/parser.hpp"
#include "indicode/sema.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

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
        "hir.indi",
        "function add(a: Int, b: Int) -> Int { return a + b }\n"
        "List<Int> empty = []\n"
        "items = [20, 22]\n"
        "answer = add(b: items[1], a: items[0])\n"
        "for item in items { say(item) }\n"
    };
    const auto lexed = indicode::Lexer(source).lex();
    require(lexed.diagnostics.empty(), "HIR fixture should lex");
    auto parsed = indicode::Parser(lexed.tokens).parse();
    require(parsed.diagnostics.empty(), "HIR fixture should parse");
    const auto semantic = indicode::Analyzer().analyze(parsed.program);
    require(semantic.diagnostics.empty(), "HIR fixture should pass semantic checks");

    auto hir = indicode::lower_to_hir(parsed.program);
    require(hir.functions.size() == 1 && hir.statements.size() == 4,
            "lowering should preserve function and top-level statement structure");
    require(indicode::verify_hir(hir).empty(), "lowered typed HIR should verify");
    const auto printed = indicode::print_hir(hir);
    require(printed.find("func @add") != std::string::npos &&
                printed.find("call @add") != std::string::npos &&
                printed.find("b: ") != std::string::npos &&
                printed.find("list<List<Int>>") != std::string::npos &&
                printed.find("for %item: Int in") != std::string::npos,
            "typed HIR printer should show functions, collections, loops, and types");

    auto missing_return_hir = indicode::lower_to_hir(parsed.program);
    missing_return_hir.functions[0].body.clear();
    const auto missing_return_diagnostics = indicode::verify_hir(missing_return_hir);
    require(std::any_of(missing_return_diagnostics.begin(), missing_return_diagnostics.end(),
                        [](const auto& diagnostic) { return diagnostic.code == "E-HIR-026"; }),
            "HIR verifier should independently reject a non-Void function without complete return paths");

    auto duplicate_parameter_hir = indicode::lower_to_hir(parsed.program);
    duplicate_parameter_hir.functions[0].parameters[1].name =
        duplicate_parameter_hir.functions[0].parameters[0].name;
    const auto duplicate_parameter_diagnostics = indicode::verify_hir(duplicate_parameter_hir);
    require(std::any_of(duplicate_parameter_diagnostics.begin(), duplicate_parameter_diagnostics.end(),
                        [](const auto& diagnostic) { return diagnostic.code == "E-HIR-030"; }),
            "HIR verifier should reject duplicate parameter names");

    auto misaligned_call_hir = indicode::lower_to_hir(parsed.program);
    misaligned_call_hir.statements[2]->expression->argument_names.clear();
    const auto misaligned_call_diagnostics = indicode::verify_hir(misaligned_call_hir);
    require(std::any_of(misaligned_call_diagnostics.begin(), misaligned_call_diagnostics.end(),
                        [](const auto& diagnostic) { return diagnostic.code == "E-HIR-031"; }),
            "HIR verifier should reject call metadata that does not align with arguments");

    auto top_level_return_hir = indicode::lower_to_hir(parsed.program);
    auto top_level_return = std::make_unique<indicode::HirStmt>();
    top_level_return->kind = indicode::HirStmtKind::Return;
    top_level_return_hir.statements.push_back(std::move(top_level_return));
    const auto top_level_return_diagnostics = indicode::verify_hir(top_level_return_hir);
    require(std::any_of(top_level_return_diagnostics.begin(), top_level_return_diagnostics.end(),
                        [](const auto& diagnostic) { return diagnostic.code == "E-HIR-032"; }),
            "HIR verifier should reject return statements outside functions");

    hir.statements[0]->expression->type = "";
    const auto invalid = indicode::verify_hir(hir);
    require(!invalid.empty() && invalid[0].code == "E-HIR-004",
            "verifier should reject expressions without a valid type annotation");

    const indicode::SourceFile records_source{
        "records.indi",
        "record Point { x: Int, y: Int }\n"
        "origin = Point(x: 20, y: 22)\n"
        "answer = origin.x\n"
        "small = to_Int2(20, checked: true)\n"
        "weight = to_Float(0.5, checked: true)\n"
        "for index in 1..4 { say(index) }\n"
    };
    const auto records_lexed = indicode::Lexer(records_source).lex();
    auto records_parsed = indicode::Parser(records_lexed.tokens).parse();
    require(records_lexed.diagnostics.empty() && records_parsed.diagnostics.empty(),
            "record HIR fixture should lex and parse");
    require(indicode::Analyzer().analyze(records_parsed.program).diagnostics.empty(),
            "record HIR fixture should pass semantic checks");
    const auto records_hir = indicode::lower_to_hir(records_parsed.program);
    require(records_hir.records.size() == 1 && indicode::verify_hir(records_hir).empty(),
            "typed HIR should preserve and verify record declarations and access");
    const auto records_printed = indicode::print_hir(records_hir);
    require(records_printed.find("record @Point") != std::string::npos &&
                records_printed.find(".x : Int") != std::string::npos &&
                records_printed.find("call @to_Int2") != std::string::npos &&
                records_printed.find("call @to_Float") != std::string::npos &&
                records_printed.find("range 1:Int .. 4:Int : Range<Int>") != std::string::npos,
            "typed HIR printer should show records, member access, conversions, and ranges");

    const indicode::SourceFile tensor_source{
        "tensor.indi",
        "function preserve(input: Tensor<Float, [Batch, 3]>) -> Tensor<Float, [Batch, 3]> "
        "{ return input }\n"
    };
    const auto tensor_lexed = indicode::Lexer(tensor_source).lex();
    auto tensor_parsed = indicode::Parser(tensor_lexed.tokens).parse();
    require(tensor_lexed.diagnostics.empty() && tensor_parsed.diagnostics.empty() &&
                indicode::Analyzer().analyze(tensor_parsed.program).diagnostics.empty(),
            "tensor HIR fixture should parse and pass semantic checks");
    const auto tensor_hir = indicode::lower_to_hir(tensor_parsed.program);
    require(indicode::verify_hir(tensor_hir).empty(),
            "typed HIR should preserve and verify tensor shape annotations");

    const indicode::SourceFile else_if_source{
        "else_if.indi",
        "function classify(value: Int) -> Int {\n"
        "  if value == 0 { return 0 } else if value == 1 { return 1 } else { return 2 }\n"
        "}\n"
    };
    const auto else_if_lexed = indicode::Lexer(else_if_source).lex();
    auto else_if_parsed = indicode::Parser(else_if_lexed.tokens).parse();
    require(else_if_lexed.diagnostics.empty() && else_if_parsed.diagnostics.empty() &&
                indicode::Analyzer().analyze(else_if_parsed.program).diagnostics.empty(),
            "else-if chain should pass parsing and semantic analysis");
    const auto else_if_hir = indicode::lower_to_hir(else_if_parsed.program);
    require(indicode::verify_hir(else_if_hir).empty(),
            "else-if chain should lower to valid HIR with complete return coverage");

    std::cout << "HIR tests passed\n";
}