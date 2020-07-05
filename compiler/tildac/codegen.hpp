#ifndef CRUST_CODEGEN_HPP
#define CRUST_CODEGEN_HPP

#include <tildac/ast.hpp>

namespace tildac {
    void generate(const AST_Node& node, const bool optimize);
} // namespace tildac

#endif //CRUST_CODEGEN_HPP
