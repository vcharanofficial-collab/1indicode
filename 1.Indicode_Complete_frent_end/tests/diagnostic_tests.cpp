#include "indicode/diagnostic.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}

int main() {
    const indicode::SourceFile source{"sample\".indi", "Int x = \"bad\"\n"};
    const indicode::Diagnostic diagnostic{
        indicode::DiagnosticSeverity::Error,
        "E-TYPE-014",
        "cannot assign \"bad\" to Int\ncheck the value",
        {{8, 1, 9}, {13, 1, 14}}
    };

    const auto text = indicode::format_diagnostic_text(source, diagnostic);
    require(text.find("sample\".indi:1:9: error[E-TYPE-014]") != std::string::npos,
            "text diagnostic should include path, location, severity, and stable code");
    require(text.find("Int x = \"bad\"") != std::string::npos &&
                text.find("^^^^^") != std::string::npos,
            "text diagnostic should include the source line and underline");

    const auto json = indicode::format_diagnostics_json(source, {diagnostic});
    require(json.starts_with("[{") && json.ends_with("}]"), "JSON diagnostics should be an array");
    require(json.find("sample\\\".indi") != std::string::npos,
            "JSON path should escape quotation marks");
    require(json.find("cannot assign \\\"bad\\\" to Int\\ncheck the value") != std::string::npos,
            "JSON message should escape quotes and line breaks");
    require(json.find("\"start\":{\"offset\":8,\"line\":1,\"column\":9}") != std::string::npos,
            "JSON should preserve structured start position");

    const indicode::SourceFile unicode_source{"unicode.indi", "\tπ = value\n"};
    const indicode::Diagnostic unicode_diagnostic{
        indicode::DiagnosticSeverity::Error,
        "E-TEST-001",
        "unknown name",
        {{6, 1, 7}, {11, 1, 12}}
    };
    const auto unicode_text = indicode::format_diagnostic_text(unicode_source, unicode_diagnostic);
    require(unicode_text.find("    π = value\n          ^^^^^") != std::string::npos,
            "diagnostic caret should align after tabs and multi-byte UTF-8 scalars");

    std::cout << "Diagnostic tests passed\n";
}