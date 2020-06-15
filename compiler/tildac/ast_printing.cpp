#include <tildac/ast_printing.hpp>

#include <tildac/ast.hpp>

#include <iostream>

namespace tildac {
    struct Indent {
        i64 indent_count = 0;
    };

    static std::ostream& operator<<(std::ostream& stream, Indent indent) {
        for(i64 i = 0; i < indent.indent_count; ++i) {
            stream << "  ";
        }
        return stream;
    }

    void print_ast(AST_Node const& ast_node, i64 indent_level) {
        switch(ast_node.node_type) {
            case AST_Node_Type::identifier: {
                Identifier& node = (Identifier&)ast_node;
                std::cout << Indent{indent_level} << "Identifier: '" << node.name << "'\n";
                return;
            }

            case AST_Node_Type::qualified_type: {
                Qualified_Type& node = (Qualified_Type&)ast_node;
                std::cout << Indent{indent_level} << "Qualified_Type: '" << node.name << "'\n";
                return;
            }

            case AST_Node_Type::template_id: {
                Template_ID& node = (Template_ID&)ast_node;
                std::cout << Indent{indent_level} << "Template_ID:\n";
                std::cout << Indent{indent_level + 1} << "Type:\n";
                print_ast(*node.qualified_type, indent_level + 2);
                std::cout << Indent{indent_level + 1} << "Nested Types:\n";
                for(auto& nested_type: node.nested_types) {
                    print_ast(*nested_type, indent_level + 2);
                }
                return;
            }

            case AST_Node_Type::identifier_expression: {
                Identifier_Expression& node = (Identifier_Expression&)ast_node;
                std::cout << Indent{indent_level} << "Identifier_Expression:\n";
                print_ast(*node.identifier, indent_level + 1);
                return;
            }

            case AST_Node_Type::boolean_or_expression: {
                std::cout << Indent{indent_level} << "Boolean_Or_Expression:\n";
                print_ast(*node.lhs, indent_level + 1);
                print_ast(*node.rhs, indent_level + 1);
            }

            case AST_Node_Type::statement_list: {
                Statement_List& node = (Statement_List&)ast_node;
                for(auto& statement: node.statements) {
                    print_ast(*statement, indent_level);
                }
                return;
            }
        }
    }
} // namespace tildac
