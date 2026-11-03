#pragma once

#include "indicode/source.hpp"

#include <string>
#include <vector>

namespace indicode {

enum class DiagnosticSeverity {
    Error,
    Warning
};

struct Diagnostic {
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string code;
    std::string message;
    SourceSpan span;
};

std::string format_diagnostic_text(const SourceFile& source, const Diagnostic& diagnostic);
std::string format_diagnostics_json(const SourceFile& source,
                                    const std::vector<Diagnostic>& diagnostics);

}