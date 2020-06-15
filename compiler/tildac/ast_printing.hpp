#pragma once

namespace tildac {
    struct AST_Node;

    void print_ast(AST_Node const& node, i64 indent_level);
} // namespace tildac
