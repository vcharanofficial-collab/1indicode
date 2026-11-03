#include "indicode/hir.hpp"
#include "indicode/lexer.hpp"
#include "indicode/parser.hpp"
#include "indicode/sema.hpp"

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
                records_printed.find("call @to_Int2") != std::string::npos,
            "typed HIR printer should show records, member access, and checked conversions");

    std::cout << "HIR tests passed\n";
}