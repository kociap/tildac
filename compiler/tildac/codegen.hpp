#ifndef CRUST_CODEGEN_HPP
#define CRUST_CODEGEN_HPP

#include <tildac/utility.hpp>
#include <tildac/ast.hpp>

namespace tildac {
    void generate(const std::vector<Owning_Ptr<AST_Node>>& nodes, const bool optimize);
} // namespace tildac

#endif //CRUST_CODEGEN_HPP
