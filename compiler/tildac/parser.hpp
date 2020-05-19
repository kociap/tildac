#pragma once

#include <tildac/types.hpp>
#include <string>
#include <string_view>

namespace tildac {
    struct Parse_Error {
        std::string message;
        i64 line;
        i64 column;
    };

    void parse_file(std::string_view path);
}
