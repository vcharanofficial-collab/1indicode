#pragma once

#include <cstddef>
#include <string>

namespace indicode {

struct SourceLocation {
    std::size_t offset = 0;
    std::size_t line = 1;
    std::size_t column = 1;
};

struct SourceSpan {
    SourceLocation start;
    SourceLocation end;
};

struct SourceFile {
    std::string name;
    std::string contents;
};

}