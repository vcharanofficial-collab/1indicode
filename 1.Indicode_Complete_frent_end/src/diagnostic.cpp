#include "indicode/diagnostic.hpp"

#include <algorithm>
#include <sstream>
#include <string_view>

namespace indicode {
namespace {

const char* severity_name(DiagnosticSeverity severity) {
    return severity == DiagnosticSeverity::Error ? "error" : "warning";
}

std::string json_escape(std::string_view value) {
    std::string result;
    result.reserve(value.size() + 2);
    for (const unsigned char character : value) {
        switch (character) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (character < 0x20) {
                constexpr char hex[] = "0123456789abcdef";
                result += "\\u00";
                result += hex[(character >> 4) & 0x0f];
                result += hex[character & 0x0f];
            } else {
                result += static_cast<char>(character);
            }
        }
    }
    return result;
}

std::string_view source_line(const SourceFile& source, std::size_t requested_line) {
    std::size_t line = 1;
    std::size_t start = 0;
    for (std::size_t index = 0; index < source.contents.size(); ++index) {
        if (line == requested_line) {
            start = index;
            break;
        }
        if (source.contents[index] == '\n') {
            ++line;
            start = index + 1;
        }
    }
    if (line != requested_line) {
        if (requested_line == line) {
            start = source.contents.size();
        } else {
            return {};
        }
    }
    auto end = source.contents.find_first_of("\r\n", start);
    if (end == std::string::npos) {
        end = source.contents.size();
    }
    return std::string_view(source.contents).substr(start, end - start);
}

}

std::string format_diagnostic_text(const SourceFile& source, const Diagnostic& diagnostic) {
    std::ostringstream output;
    output << source.name << ':' << diagnostic.span.start.line << ':'
           << diagnostic.span.start.column << ": " << severity_name(diagnostic.severity)
           << '[' << diagnostic.code << "]: " << diagnostic.message;

    const auto text = source_line(source, diagnostic.span.start.line);
    if (text.empty()) {
        return output.str();
    }
    output << "\n  " << text << "\n  ";
    const auto column = diagnostic.span.start.column > 0 ? diagnostic.span.start.column - 1 : 0;
    output << std::string(std::min(column, text.size()), ' ');
    const auto same_line_end = diagnostic.span.end.line == diagnostic.span.start.line
        ? diagnostic.span.end.column
        : diagnostic.span.start.column + 1;
    const auto width = same_line_end > diagnostic.span.start.column
        ? same_line_end - diagnostic.span.start.column
        : 1;
    output << std::string(std::min(width, text.size() > column ? text.size() - column : std::size_t{1}), '^');
    return output.str();
}

std::string format_diagnostics_json(const SourceFile& source,
                                    const std::vector<Diagnostic>& diagnostics) {
    std::ostringstream output;
    output << '[';
    for (std::size_t index = 0; index < diagnostics.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        const auto& diagnostic = diagnostics[index];
        output << "{\"severity\":\"" << severity_name(diagnostic.severity)
               << "\",\"code\":\"" << json_escape(diagnostic.code)
               << "\",\"message\":\"" << json_escape(diagnostic.message)
               << "\",\"file\":\"" << json_escape(source.name)
               << "\",\"start\":{\"offset\":" << diagnostic.span.start.offset
               << ",\"line\":" << diagnostic.span.start.line
               << ",\"column\":" << diagnostic.span.start.column
               << "},\"end\":{\"offset\":" << diagnostic.span.end.offset
               << ",\"line\":" << diagnostic.span.end.line
               << ",\"column\":" << diagnostic.span.end.column << "}}";
    }
    output << ']';
    return output.str();
}

}