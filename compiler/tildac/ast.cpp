#include <tildac/ast.hpp>

namespace tildac {
    std::ostream& operator<<(std::ostream& stream, Indent indent) {
        for(i64 i = 0; i < indent.indent_count; ++i) {
            stream << "  ";
        }
        return stream;
    }

    void Statement_List::print(std::ostream& stream, Indent const indent) const {
        for(auto& statement: _statements) {
            statement->print(stream, indent);
        }
    }
}
