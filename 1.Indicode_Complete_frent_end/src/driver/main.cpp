#include "indicode/ast.hpp"
#include "indicode/hir.hpp"
#include "indicode/lexer.hpp"
#include "indicode/parser.hpp"
#include "indicode/sema.hpp"
#include "indicode/token.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

void print_diagnostic(const indicode::SourceFile& source, const indicode::Diagnostic& diagnostic) {
    const auto severity = diagnostic.severity == indicode::DiagnosticSeverity::Error ? "error" : "warning";
    std::cerr << source.name << ':' << diagnostic.span.start.line << ':'
              << diagnostic.span.start.column << ": " << severity << '[' << diagnostic.code
              << "]: " << diagnostic.message << '\n';
}

void print_diagnostics(const indicode::SourceFile& source,
                       const std::vector<indicode::Diagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        print_diagnostic(source, diagnostic);
    }
}

}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: indicode <check|tokens|ast|typed> <file.indi>\n";
        return 2;
    }

    const std::string command = argv[1];
    if (command != "check" && command != "tokens" && command != "ast" && command != "typed") {
        std::cerr << "unknown command: " << command << '\n';
        return 2;
    }

    indicode::SourceFile source;
    source.name = argv[2];
    std::ifstream input(source.name, std::ios::binary);
    if (!input) {
        std::cerr << "cannot open source file: " << source.name << '\n';
        return 2;
    }
    source.contents.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());

    auto lexed = indicode::Lexer(source).lex();
    if (command == "tokens") {
        for (const auto& token : lexed.tokens) {
            std::cout << token.span.start.line << ':' << token.span.start.column << ' '
                      << indicode::token_kind_name(token.kind);
            if (!token.lexeme.empty()) {
                std::cout << " `" << token.lexeme << '`';
            }
            std::cout << '\n';
        }
        print_diagnostics(source, lexed.diagnostics);
        return lexed.diagnostics.empty() ? 0 : 1;
    }
    if (!lexed.diagnostics.empty()) {
        print_diagnostics(source, lexed.diagnostics);
        return 1;
    }

    auto parsed = indicode::Parser(lexed.tokens).parse();
    if (!parsed.diagnostics.empty()) {
        print_diagnostics(source, parsed.diagnostics);
        return 1;
    }
    if (command == "ast") {
        std::cout << indicode::print_program(parsed.program);
        return 0;
    }

    auto analyzed = indicode::Analyzer().analyze(parsed.program);
    if (!analyzed.diagnostics.empty()) {
        print_diagnostics(source, analyzed.diagnostics);
        return 1;
    }
    auto hir = indicode::lower_to_hir(parsed.program);
    const auto hir_diagnostics = indicode::verify_hir(hir);
    if (!hir_diagnostics.empty()) {
        print_diagnostics(source, hir_diagnostics);
        return 1;
    }
    if (command == "typed") {
        std::cout << indicode::print_hir(hir);
    } else {
        std::cout << "ok: " << source.name << '\n';
    }
    return 0;
}