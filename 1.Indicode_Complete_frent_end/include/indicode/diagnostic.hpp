#pragma once

#include "indicode/source.hpp"

#include <string>

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

}