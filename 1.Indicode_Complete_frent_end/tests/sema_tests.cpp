#include "indicode/lexer.hpp"
#include "indicode/parser.hpp"
#include "indicode/sema.hpp"

#include <cstdlib>
#include <iostream>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

indicode::Program parse_source(const indicode::SourceFile& source) {
    const auto lexed = indicode::Lexer(source).lex();
    require(lexed.diagnostics.empty(), "semantic fixture should lex cleanly");
    auto parsed = indicode::Parser(lexed.tokens).parse();
    require(parsed.diagnostics.empty(), "semantic fixture should parse cleanly");
    return std::move(parsed.program);
}

}

int main() {
    const indicode::SourceFile valid{
        "valid.indi",
        "function add(a: Int, b: Int) -> Int { return a + b }\n"
        "let fixed: Int = 3\n"
        "Int2 small = 30000\n"
        "Int2 minimum = -32768\n"
        "counter = add(fixed, 4)\n"
        "counter = counter + 1\n"
        "say(\"ok\")\n"
    };
    auto valid_program = parse_source(valid);
    const auto valid_result = indicode::Analyzer().analyze(valid_program);
    require(valid_result.diagnostics.empty(), "valid declarations and calls should type-check");
        require(valid_program.statements[3]->declared_type == "Int",
            "inferred assignment should retain its resolved static type");
        require(valid_program.statements[3]->expression->inferred_type == "Int",
            "expression should be annotated with inferred type");

    const indicode::SourceFile invalid{
        "invalid.indi",
        "let fixed: Int = 3\n"
        "Int2 tooLarge = 40000\n"
        "Int2 tooSmall = -32769\n"
        "fixed = 4\n"
        "count = \"wrong\"\n"
        "missing = unknown_name\n"
        "function broken(value: Int) -> Bool { return value }\n"
    };
    auto invalid_program = parse_source(invalid);
    const auto invalid_result = indicode::Analyzer().analyze(invalid_program);
    require(invalid_result.diagnostics.size() >= 5,
            "range, immutable writes, unknown names, and mismatched returns should be diagnosed");
    bool saw_range_error = false;
    for (const auto& diagnostic : invalid_result.diagnostics) {
        saw_range_error = saw_range_error || diagnostic.code == "E-SEMA-022";
    }
    require(saw_range_error, "out-of-range Int2 literals should use the stable range diagnostic");

    std::cout << "Semantic tests passed\n";
}