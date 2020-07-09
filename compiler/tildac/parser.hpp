#pragma once

#include <anton/expected.hpp>
#include <string>
#include <string_view>
#include <tildac/types.hpp>

namespace tildac {
    struct Parse_Error {
        std::string message;
        i64 line;
        i64 column;
        i64 file_offset;
    };

    anton::Expected<Owning_Ptr<Declaration_Sequence>, Parse_Error> parse_file(std::string_view path);
} // namespace tildac
