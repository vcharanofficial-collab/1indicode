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
            "Int2 hexLimit = 0x7fff\n"
            "UInt2 binaryLimit = 0b1111111111111111\n"
            "Long grouped = 1_000_000\n"
        "Int2 minimum = -32768\n"
        "List<Int> empty = []\n"
        "counter = add(fixed, 4)\n"
        "counter = counter + 1\n"
        "items = [1, 2, 3]\n"
        "first = items[0]\n"
        "for item in items { counter = counter + item }\n"
        "for character in \"ok\" { say(character) }\n"
        "accent = \"\xC3\xA9\"\n"
        "escaped = \"\\u{1F600}\"\n"
        "combined = \"e\\u{301}\"\n"
        "namedAnswer = add(b: fixed, a: 4)\n"
        "say(\"ok\")\n"
    };
    auto valid_program = parse_source(valid);
    const auto valid_result = indicode::Analyzer().analyze(valid_program);
    require(valid_result.diagnostics.empty(), "valid declarations and calls should type-check");
        require(valid_program.statements[6]->expression->inferred_type == "List<Int>",
            "an empty list should receive its declared element type");
                require(valid_program.statements[7]->declared_type == "Int",
            "inferred assignment should retain its resolved static type");
                require(valid_program.statements[7]->expression->inferred_type == "Int",
            "expression should be annotated with inferred type");
                require(valid_program.statements[10]->expression->inferred_type == "Int",
                "list indexing should infer the list element type");
                require(valid_program.statements[11]->declared_type == "Int" &&
                    valid_program.statements[12]->declared_type == "Char",
                "for-each should infer list and text element types");
                require(valid_program.statements[13]->declared_type == "Char" &&
                    valid_program.statements[14]->declared_type == "Char" &&
                    valid_program.statements[15]->declared_type == "Text",
                    "string literals should infer Char/Text by Unicode scalar count");

    const indicode::SourceFile invalid{
        "invalid.indi",
        "function add(a: Int, b: Int) -> Int { return a + b }\n"
        "let fixed: Int = 3\n"
        "mixed = [1, \"two\"]\n"
        "invalid_index = mixed[false]\n"
        "for item in 5 { say(item) }\n"
        "unknown_named = add(c: 1, a: 2)\n"
        "duplicate_named = add(a: 1, a: 2)\n"
        "Int2 radixTooLarge = 0x8000\n"
        "Int2 tooLarge = 40000\n"
        "Int2 tooSmall = -32769\n"
        "fixed = 4\n"
        "count = \"wrong\"\n"
        "missing = unknown_name\n"
        "function broken(value: Int) -> Bool { return value }\n"
    };
    auto invalid_program = parse_source(invalid);
    const auto invalid_result = indicode::Analyzer().analyze(invalid_program);
        require(invalid_result.diagnostics.size() >= 8,
            "invalid lists/indexes/iterables and prior type errors should be diagnosed");
    bool saw_range_error = false;
    for (const auto& diagnostic : invalid_result.diagnostics) {
        saw_range_error = saw_range_error || diagnostic.code == "E-SEMA-022";
    }
    require(saw_range_error, "out-of-range Int2 literals should use the stable range diagnostic");
    bool saw_unknown_named = false;
    bool saw_duplicate_named = false;
    for (const auto& diagnostic : invalid_result.diagnostics) {
        saw_unknown_named = saw_unknown_named || diagnostic.code == "E-SEMA-028";
        saw_duplicate_named = saw_duplicate_named || diagnostic.code == "E-SEMA-030";
    }
    require(saw_unknown_named && saw_duplicate_named,
            "unknown and duplicate named arguments should be rejected");

    std::cout << "Semantic tests passed\n";
}