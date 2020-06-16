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
                auto const& node = static_cast<Identifier const&>(ast_node);
                std::cout << Indent{indent_level} << "Identifier: '" << node.name << "'\n";
                return;
            }

            case AST_Node_Type::qualified_type: {
                auto const& node = static_cast<Qualified_Type const&>(ast_node);
                std::cout << Indent{indent_level} << "Qualified_Type: '" << node.name << "'\n";
                return;
            }

            case AST_Node_Type::template_id: {
                auto const& node = static_cast<Template_ID const&>(ast_node);
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
                auto const& node = static_cast<Identifier_Expression const&>(ast_node);
                std::cout << Indent{indent_level} << "Identifier_Expression:\n";
                print_ast(*node.identifier, indent_level + 1);
                return;
            }

            case AST_Node_Type::boolean_or_expression: {
                auto const& node = static_cast<Boolean_Or_Expression const&>(ast_node);
                std::cout << Indent{indent_level} << "Boolean_Or_Expression:\n";
                print_ast(*node.lhs, indent_level + 1);
                print_ast(*node.rhs, indent_level + 1);
                return;
            }

            case AST_Node_Type::statement_list: {
                auto const& node = static_cast<Statement_List const&>(ast_node);
                for(auto& statement: node.statements) {
                    print_ast(*statement, indent_level);
                }
                return;
            }

            case AST_Node_Type::boolean_and_expression: {
                auto const& node = static_cast<Boolean_And_Expression const&>(ast_node);
                std::cout << Indent{indent_level} << "Boolean_And_Expression:\n";
                print_ast(*node.lhs, indent_level + 1);
                print_ast(*node.rhs, indent_level + 1);
                return;
            }

            case AST_Node_Type::add_sub_expression: {
                auto const& node = static_cast<Add_Sub_Expression const&>(ast_node);
                std::cout << Indent{indent_level} << "Add_Sub_Expression (" << (node.is_add ? "addition" : "subtraction") << "):\n";
                print_ast(*node.lhs, indent_level + 1);
                print_ast(*node.rhs, indent_level + 1);
                return;
            }

            case AST_Node_Type::mul_div_expression: {
                auto const& node = static_cast<Mul_Div_Expression const&>(ast_node);
                std::cout << Indent{indent_level} << "Mul_Div_Expression (" << (node.is_mul ? "multiplication" : "division") << "):\n";
                print_ast(*node.lhs, indent_level + 1);
                print_ast(*node.rhs, indent_level + 1);
                return;
            }

            case AST_Node_Type::argument_list: {
                auto const& node = static_cast<Argument_List const&>(ast_node);
                std::cout << Indent{indent_level} << "Argument_List:\n";
                for(const auto& argument: node.arguments) {
                    print_ast(*argument, indent_level + 1);
                }
                return;
            }

            case AST_Node_Type::function_call_expression: {
                auto const& node = static_cast<Function_Call_Expression const&>(ast_node);
                std::cout << Indent{indent_level} << "Function_Call_Expression:\n";
                print_ast(*node.identifier, indent_level + 1);
                print_ast(*node.arg_list, indent_level + 1);
                return;
            }

            case AST_Node_Type::bool_literal: {
                auto const& node = static_cast<Bool_Literal const&>(ast_node);
                std::cout << Indent{indent_level} << "Bool_Literal:\n";
                std::cout << Indent{indent_level + 1} << "Value: " << std::boolalpha << node.value << std::noboolalpha << "\n";
                return;
            }

            case AST_Node_Type::integer_literal: {
                auto const& node = static_cast<Integer_Literal const&>(ast_node);
                std::cout << Indent{indent_level} << "Integer_Literal:\n";
                std::cout << Indent{indent_level + 1} << "Value: " << node.value << "\n";
                return;
            }

            case AST_Node_Type::declaration_sequence: {
                auto const& node = static_cast<Declaration_Sequence const&>(ast_node);
                std::cout << Indent{indent_level} << "Declaration_Sequence:\n";
                for(const auto& decl: node.decls) {
                    print_ast(*decl, indent_level + 1);
                }
                return;
            }

            case AST_Node_Type::variable_declaration: {
                auto const& node = static_cast<Variable_Declaration const&>(ast_node);
                std::cout << Indent{indent_level} << "Variable_Declaration:\n";
                print_ast(*node.identifier, indent_level + 1);
                print_ast(*node.type, indent_level + 1);
                if(node.initializer) {
                    print_ast(*node.initializer, indent_level + 1);
                }
                return;
            }

            case AST_Node_Type::block_statement: {
                auto const& node = static_cast<Block_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "Block_Statement:\n";
                print_ast(*node.statements, indent_level + 1);
                return;
            }

            case AST_Node_Type::if_statement: {
                auto const& node = static_cast<If_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "If_Statement:\n";
                print_ast(*node.block, indent_level + 1);
                print_ast(*node.condition, indent_level + 1);
                return;
            }

            case AST_Node_Type::while_statement: {
                auto const& node = static_cast<While_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "While_Statement:\n";
                print_ast(*node.block, indent_level + 1);
                print_ast(*node.condition, indent_level + 1);
                return;
            }

            case AST_Node_Type::do_while_statement: {
                auto const& node = static_cast<Do_While_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "Do_While_Statement:\n";
                print_ast(*node.block, indent_level + 1);
                print_ast(*node.condition, indent_level + 1);
                return;
            }

            case AST_Node_Type::return_statement: {
                auto const& node = static_cast<Return_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "Return_Statement:\n";
                print_ast(*node.expression, indent_level + 1);
                return;
            }

            case AST_Node_Type::declaration_statement: {
                auto const& node = static_cast<Declaration_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "Declaration_Statement:\n";
                print_ast(*node.var_decl, indent_level + 1);
                return;
            }

            case AST_Node_Type::expression_statement: {
                auto const& node = static_cast<Expression_Statement const&>(ast_node);
                std::cout << Indent{indent_level} << "Expression_Statement:\n";
                print_ast(*node.expr, indent_level + 1);
                return;
            }

            case AST_Node_Type::function_parameter: {
                auto const& node = static_cast<Function_Parameter const&>(ast_node);
                std::cout << Indent{indent_level} << "Function_Parameter:\n";
                print_ast(*node.identifier, indent_level + 1);
                print_ast(*node.type, indent_level + 1);
                return;
            }

            case AST_Node_Type::function_parameter_list: {
                auto const& node = static_cast<Function_Parameter_List const&>(ast_node);
                std::cout << Indent{indent_level} << "Function_Parameter_List:\n";
                for(const auto& param: node.params) {
                    print_ast(*param, indent_level + 1);
                }
                return;
            }

            case AST_Node_Type::function_body: {
                auto const& node = static_cast<Function_Body const&>(ast_node);
                std::cout << Indent{indent_level} << "Function_Body:\n";
                print_ast(*node.statements, indent_level + 1);
                return;
            }

            case AST_Node_Type::function_declaration: {
                auto const& node = static_cast<Function_Declaration const&>(ast_node);
                std::cout << Indent{indent_level} << "Function_Declaration:\n";
                std::cout << Indent{indent_level + 1} << "Function Name:\n";
                print_ast(*node.name, indent_level + 2);
                std::cout << Indent{indent_level + 1} << "Return Type:\n";
                print_ast(*node.return_type, indent_level + 2);
                print_ast(*node.parameter_list, indent_level + 1);
                print_ast(*node.body, indent_level + 1);
                return;
            }
        }
    }
} // namespace tildac
